/* Linuxthreads - a simple clone()-based implementation of Posix        */
/* threads for Linux.                                                   */
/* Copyright (C) 1996 Xavier Leroy (Xavier.Leroy@inria.fr)              */
/*                                                                      */
/* This program is free software; you can redistribute it and/or        */
/* modify it under the terms of the GNU Library General Public License  */
/* as published by the Free Software Foundation; either version 2       */
/* of the License, or (at your option) any later version.               */
/*                                                                      */
/* This program is distributed in the hope that it will be useful,      */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of       */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        */
/* GNU Library General Public License for more details.                 */

/* The "thread manager" thread: manages creation and termination of threads */

#include <errno.h>
#include <sched.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/poll.h>		/* for poll */
#include <sys/mman.h>           /* for mmap */
#include <sys/param.h>
#include <sys/time.h>
#include <sys/wait.h>           /* for waitpid macros */
#include <linux/tasks.h>

#include "pthread.h"
#include "internals.h"
#include "spinlock.h"
#include "restart.h"
#include "semaphore.h"

/* Array of active threads. Entry 0 is reserved for the initial thread. */
struct pthread_handle_struct __pthread_handles[PTHREAD_THREADS_MAX] =
{ { LOCK_INITIALIZER, &__pthread_initial_thread, 0},
  { LOCK_INITIALIZER, &__pthread_manager_thread, 0}, /* All NULLs */ };

/* Indicate whether at least one thread has a user-defined stack (if 1),
   or if all threads have stacks supplied by LinuxThreads (if 0). */
int __pthread_nonstandard_stacks = 0;

/* Number of active entries in __pthread_handles (used by gdb) */
volatile int __pthread_handles_num = 2;

/* Whether to use debugger additional actions for thread creation
   (set to 1 by gdb) */
volatile int __pthread_threads_debug = 0;

/* Mapping from stack segment to thread descriptor. */
/* Stack segment numbers are also indices into the __pthread_handles array. */
/* Stack segment number 0 is reserved for the initial thread. */

static inline pthread_descr thread_segment(int seg)
{
  return (pthread_descr)(THREAD_STACK_START_ADDRESS - (seg - 1) * STACK_SIZE)
         - 1;
}

/* Flag set in signal handler to record child termination */

static volatile int terminated_children = 0;

/* Flag set when the initial thread is blocked on pthread_exit waiting
   for all other threads to terminate */

static int main_thread_exiting = 0;

/* Counter used to generate unique thread identifier.
   Thread identifier is pthread_threads_counter + segment. */

static pthread_t pthread_threads_counter = 0;

/* Forward declarations */

static int pthread_handle_create(pthread_t *thread, const pthread_attr_t *attr,
                                 void * (*start_routine)(void *), void *arg,
                                 sigset_t *mask, int father_pid);
static void pthread_handle_free(pthread_t th_id);
static void pthread_handle_exit(pthread_descr issuing_thread, int exitcode);
static void pthread_reap_children(void);
static void pthread_kill_all_threads(int sig, int main_thread_also);

/* The server thread managing requests for thread creation and termination */

int __pthread_manager(void *arg)
{
  int reqfd = (int)arg;
  struct pollfd ufd;
  sigset_t mask;
  int n;
  struct pthread_request request;

  /* If we have special thread_self processing, initialize it.  */
#ifdef INIT_THREAD_SELF
  INIT_THREAD_SELF(&__pthread_manager_thread, 1);
#endif
  /* Set the error variable.  */
  __pthread_manager_thread.p_errnop = &__pthread_manager_thread.p_errno;
  __pthread_manager_thread.p_h_errnop = &__pthread_manager_thread.p_h_errno;
  /* Block all signals except __pthread_sig_cancel and SIGTRAP */
  sigfillset(&mask);
  sigdelset(&mask, __pthread_sig_cancel); /* for thread termination */
  sigdelset(&mask, SIGTRAP);            /* for debugging purposes */
  sigprocmask(SIG_SETMASK, &mask, NULL);
  /* Raise our priority to match that of main thread */
  __pthread_manager_adjust_prio(__pthread_main_thread->p_priority);
  /* Synchronize debugging of the thread manager */
  n = __libc_read(reqfd, (char *)&request, sizeof(request));
  ASSERT(n == sizeof(request) && request.req_kind == REQ_DEBUG);
  ufd.fd = reqfd;
  ufd.events = POLLIN;
  /* Enter server loop */
  while(1) {
    n = __poll(&ufd, 1, 2000);

    /* Check for termination of the main thread */
    if (getppid() == 1) {
      pthread_kill_all_threads(SIGKILL, 0);
      _exit(0);
    }
    /* Check for dead children */
    if (terminated_children) {
      terminated_children = 0;
      pthread_reap_children();
    }
    /* Read and execute request */
    if (n == 1 && (ufd.revents & POLLIN)) {
      n = __libc_read(reqfd, (char *)&request, sizeof(request));
      ASSERT(n == sizeof(request));
      switch(request.req_kind) {
      case REQ_CREATE:
        request.req_thread->p_retcode =
          pthread_handle_create((pthread_t *) &request.req_thread->p_retval,
                                request.req_args.create.attr,
                                request.req_args.create.fn,
                                request.req_args.create.arg,
                                &request.req_args.create.mask,
                                request.req_thread->p_pid);
        restart(request.req_thread);
        break;
      case REQ_FREE:
	pthread_handle_free(request.req_args.free.thread_id);
        break;
      case REQ_PROCESS_EXIT:
        pthread_handle_exit(request.req_thread,
                            request.req_args.exit.code);
        break;
      case REQ_MAIN_THREAD_EXIT:
        main_thread_exiting = 1;
        if (__pthread_main_thread->p_nextlive == __pthread_main_thread) {
          restart(__pthread_main_thread);
          return 0;
        }
        break;
      case REQ_POST:
        sem_post(request.req_args.post);
        break;
      case REQ_DEBUG:
        /* Make gdb aware of new thread */
	if (__pthread_threads_debug) raise(__pthread_sig_cancel);
        restart(request.req_thread);
        break;
      }
    }
  }
}

/* Process creation */

static int pthread_start_thread(void *arg)
{
  pthread_descr self = (pthread_descr) arg;
  struct pthread_request request;
  void * outcome;
  /* Initialize special thread_self processing, if any.  */
#ifdef INIT_THREAD_SELF
  INIT_THREAD_SELF(self, self->p_nr);
#endif
  /* Make sure our pid field is initialized, just in case we get there
     before our father has initialized it. */
  THREAD_SETMEM(self, p_pid, __getpid());
  /* Initial signal mask is that of the creating thread. (Otherwise,
     we'd just inherit the mask of the thread manager.) */
  sigprocmask(SIG_SETMASK, &self->p_start_args.mask, NULL);
  /* Set the scheduling policy and priority for the new thread, if needed */
  if (THREAD_GETMEM(self, p_start_args.schedpolicy) >= 0)
    __sched_setscheduler(THREAD_GETMEM(self, p_pid),
			 THREAD_GETMEM(self, p_start_args.schedpolicy),
                         &self->p_start_args.schedparam);
  /* Make gdb aware of new thread */
  if (__pthread_threads_debug) {
    request.req_thread = self;
    request.req_kind = REQ_DEBUG;
    __libc_write(__pthread_manager_request,
                 (char *) &request, sizeof(request));
    suspend(self);
  }
  /* Run the thread code */
  outcome = self->p_start_args.start_routine(THREAD_GETMEM(self,
							   p_start_args.arg));
  /* Exit with the given return value */
  pthread_exit(outcome);
  return 0;
}

static int pthread_allocate_stack(const pthread_attr_t *attr,
                                  pthread_descr default_new_thread,
                                  int pagesize,
                                  pthread_descr * out_new_thread,
                                  char ** out_new_thread_bottom,
                                  char ** out_guardaddr,
                                  size_t * out_guardsize)
{
  pthread_descr new_thread;
  char * new_thread_bottom;
  char * guardaddr;
  size_t stacksize, guardsize;

  if (attr != NULL && attr->__stackaddr_set)
    {
      /* The user provided a stack. */
      new_thread =
        (pthread_descr) ((long)(attr->__stackaddr) & -sizeof(void *)) - 1;
      new_thread_bottom = (char *) attr->__stackaddr - attr->__stacksize;
      guardaddr = NULL;
      guardsize = 0;
    }
  else
    {
      /* Allocate space for stack and thread descriptor at default address */
      new_thread = default_new_thread;
      new_thread_bottom = (char *) new_thread - STACK_SIZE;
      if (mmap((caddr_t)((char *)(new_thread + 1) - INITIAL_STACK_SIZE),
               INITIAL_STACK_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC,
               MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED | MAP_GROWSDOWN,
               -1, 0) == MAP_FAILED)
        /* Bad luck, this segment is already mapped. */
        return -1;
      /* We manage to get a stack.  Now see whether we need a guard
         and allocate it if necessary.  Notice that the default
         attributes (stack_size = STACK_SIZE - pagesize and
         guardsize = pagesize) do not need a guard page, since
         the RLIMIT_STACK soft limit prevents stacks from
         running into one another. */
      if (attr == NULL ||
          attr->__guardsize == 0 ||
          (attr->__guardsize == pagesize &&
           attr->__stacksize == STACK_SIZE - pagesize))
        {
          /* We don't need a guard page. */
          guardaddr = NULL;
          guardsize = 0;
        }
      else
        {
          /* Put a bad page at the bottom of the stack */
          stacksize = roundup(attr->__stacksize, pagesize);
          if (stacksize >= STACK_SIZE - pagesize)
            stacksize = STACK_SIZE - pagesize;
          guardaddr = (void *)new_thread - stacksize;
          guardsize = attr->__guardsize;
          if (mmap ((caddr_t) guardaddr, guardsize, 0, MAP_FIXED, -1, 0)
              == MAP_FAILED)
            {
              /* We don't make this an error.  */
              guardaddr = NULL;
              guardsize = 0;
            }
        }
    }
  *out_new_thread = new_thread;
  *out_new_thread_bottom = new_thread_bottom;
  *out_guardaddr = guardaddr;
  *out_guardsize = guardsize;
  return 0;
}

static int pthread_handle_create(pthread_t *thread, const pthread_attr_t *attr,
				 void * (*start_routine)(void *), void *arg,
				 sigset_t * mask, int father_pid)
{
  size_t sseg;
  int pid;
  pthread_descr new_thread;
  char * new_thread_bottom;
  pthread_t new_thread_id;
  char *guardaddr = NULL;
  size_t guardsize = 0;
  int pagesize = __getpagesize();

  /* First check whether we have to change the policy and if yes, whether
     we can  do this.  Normally this should be done by examining the
     return value of the __sched_setscheduler call in pthread_start_thread
     but this is hard to implement.  FIXME  */
  if (attr != NULL && attr->__schedpolicy != SCHED_OTHER && geteuid () != 0)
    return EPERM;
  /* Find a free segment for the thread, and allocate a stack if needed */
  for (sseg = 2; ; sseg++)
    {
      if (sseg >= PTHREAD_THREADS_MAX)
	return EAGAIN;
      if (__pthread_handles[sseg].h_descr != NULL)
	continue;
      if (pthread_allocate_stack(attr, thread_segment(sseg), pagesize,
                                 &new_thread, &new_thread_bottom,
                                 &guardaddr, &guardsize) == 0)
        break;
    }
  /* Allocate new thread identifier */
  pthread_threads_counter += PTHREAD_THREADS_MAX;
  new_thread_id = sseg + pthread_threads_counter;
  /* Initialize the thread descriptor */
  new_thread->p_nextwaiting = NULL;
  new_thread->p_tid = new_thread_id;
  new_thread->p_priority = 0;
  new_thread->p_lock = &(__pthread_handles[sseg].h_lock);
  new_thread->p_signal = 0;
  new_thread->p_signal_jmp = NULL;
  new_thread->p_cancel_jmp = NULL;
  new_thread->p_terminated = 0;
  new_thread->p_detached = attr == NULL ? 0 : attr->__detachstate;
  new_thread->p_exited = 0;
  new_thread->p_retval = NULL;
  new_thread->p_joining = NULL;
  new_thread->p_cleanup = NULL;
  new_thread->p_cancelstate = PTHREAD_CANCEL_ENABLE;
  new_thread->p_canceltype = PTHREAD_CANCEL_DEFERRED;
  new_thread->p_canceled = 0;
  new_thread->p_errnop = &new_thread->p_errno;
  new_thread->p_errno = 0;
  new_thread->p_h_errnop = &new_thread->p_h_errno;
  new_thread->p_h_errno = 0;
  new_thread->p_in_sighandler = NULL;
  new_thread->p_sigwaiting = 0;
  new_thread->p_guardaddr = guardaddr;
  new_thread->p_guardsize = guardsize;
  new_thread->p_userstack = attr != NULL && attr->__stackaddr_set;
  memset (new_thread->p_specific, '\0',
	  PTHREAD_KEY_1STLEVEL_SIZE * sizeof (new_thread->p_specific[0]));
  new_thread->p_self = new_thread;
  new_thread->p_nr = sseg;
  /* Initialize the thread handle */
  __pthread_init_lock(&__pthread_handles[sseg].h_lock);
  __pthread_handles[sseg].h_descr = new_thread;
  __pthread_handles[sseg].h_bottom = new_thread_bottom;
  /* Determine scheduling parameters for the thread */
  new_thread->p_start_args.schedpolicy = -1;
  if (attr != NULL) {
    switch(attr->__inheritsched) {
    case PTHREAD_EXPLICIT_SCHED:
      new_thread->p_start_args.schedpolicy = attr->__schedpolicy;
      memcpy (&new_thread->p_start_args.schedparam, &attr->__schedparam,
	      sizeof (struct sched_param));
      break;
    case PTHREAD_INHERIT_SCHED:
      /* schedpolicy doesn't need to be set, only get priority */
      __sched_getparam(father_pid, &new_thread->p_start_args.schedparam);
      break;
    }
    new_thread->p_priority =
      new_thread->p_start_args.schedparam.sched_priority;
  }
  /* Finish setting up arguments to pthread_start_thread */
  new_thread->p_start_args.start_routine = start_routine;
  new_thread->p_start_args.arg = arg;
  new_thread->p_start_args.mask = *mask;
  /* Raise priority of thread manager if needed */
  __pthread_manager_adjust_prio(new_thread->p_priority);
  /* Do the cloning */
  pid = __clone(pthread_start_thread, (void **) new_thread,
                CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND |
#ifdef CLONE_PTRACE
                CLONE_PTRACE |
#endif
		__pthread_sig_cancel, new_thread);
  /* Check if cloning succeeded */
  if (pid == -1) {
    /* Free the stack if we allocated it */
    if (attr == NULL || !attr->__stackaddr_set)
      {
	munmap((caddr_t)((char *)(new_thread+1) - INITIAL_STACK_SIZE),
	       INITIAL_STACK_SIZE);
	if (new_thread->p_guardsize != 0)
	  munmap(new_thread->p_guardaddr, new_thread->p_guardsize);
      }
    __pthread_handles[sseg].h_descr = NULL;
    __pthread_handles[sseg].h_bottom = NULL;
    __pthread_handles_num--;
    return errno;
  }
  /* Insert new thread in doubly linked list of active threads */
  new_thread->p_prevlive = __pthread_main_thread;
  new_thread->p_nextlive = __pthread_main_thread->p_nextlive;
  __pthread_main_thread->p_nextlive->p_prevlive = new_thread;
  __pthread_main_thread->p_nextlive = new_thread;
  /* Set pid field of the new thread, in case we get there before the
     child starts. */
  new_thread->p_pid = pid;
  /* We're all set */
  *thread = new_thread_id;
  return 0;
}


/* Try to free the resources of a thread when requested by pthread_join
   or pthread_detach on a terminated thread. */

static void pthread_free(pthread_descr th)
{
  pthread_handle handle;
  ASSERT(th->p_exited);
  /* Make the handle invalid */
  handle =  thread_handle(th->p_tid);
  __pthread_lock(&handle->h_lock, NULL);
  handle->h_descr = NULL;
  handle->h_bottom = (char *)(-1L);
  __pthread_unlock(&handle->h_lock);
#ifdef FREE_THREAD_SELF
  FREE_THREAD_SELF(th, th->p_nr);
#endif
  /* One fewer threads in __pthread_handles */
  __pthread_handles_num--;
  /* If initial thread, nothing to free */
  if (th == &__pthread_initial_thread) return;
  if (!th->p_userstack)
    {
      /* Free the stack and thread descriptor area */
      if (th->p_guardsize != 0)
	munmap(th->p_guardaddr, th->p_guardsize);
      munmap((caddr_t) ((char *)(th+1) - STACK_SIZE), STACK_SIZE);
    }
}

/* Handle threads that have exited */

static void pthread_exited(pid_t pid)
{
  pthread_descr th;
  int detached;
  /* Find thread with that pid */
  for (th = __pthread_main_thread->p_nextlive;
       th != __pthread_main_thread;
       th = th->p_nextlive) {
    if (th->p_pid == pid) {
      /* Remove thread from list of active threads */
      th->p_nextlive->p_prevlive = th->p_prevlive;
      th->p_prevlive->p_nextlive = th->p_nextlive;
      /* Mark thread as exited, and if detached, free its resources */
      __pthread_lock(th->p_lock, NULL);
      th->p_exited = 1;
      detached = th->p_detached;
      __pthread_unlock(th->p_lock);
      if (detached)
	pthread_free(th);
      break;
    }
  }
  /* If all threads have exited and the main thread is pending on a
     pthread_exit, wake up the main thread and terminate ourselves. */
  if (main_thread_exiting &&
      __pthread_main_thread->p_nextlive == __pthread_main_thread) {
    restart(__pthread_main_thread);
    _exit(0);
  }
}

static void pthread_reap_children(void)
{
  pid_t pid;
  int status;

  while ((pid = __libc_waitpid(-1, &status, WNOHANG | __WCLONE)) > 0) {
    pthread_exited(pid);
    if (WIFSIGNALED(status)) {
      /* If a thread died due to a signal, send the same signal to
         all other threads, including the main thread. */
      pthread_kill_all_threads(WTERMSIG(status), 1);
      _exit(0);
    }
  }
}

/* Try to free the resources of a thread when requested by pthread_join
   or pthread_detach on a terminated thread. */

static void pthread_handle_free(pthread_t th_id)
{
  pthread_handle handle = thread_handle(th_id);
  pthread_descr th;

  __pthread_lock(&handle->h_lock, NULL);
  if (invalid_handle(handle, th_id)) {
    /* pthread_reap_children has deallocated the thread already,
       nothing needs to be done */
    __pthread_unlock(&handle->h_lock);
    return;
  }
  th = handle->h_descr;
  if (th->p_exited) {
    __pthread_unlock(&handle->h_lock);
    pthread_free(th);
  } else {
    /* The Unix process of the thread is still running.
       Mark the thread as detached so that the thread manager will
       deallocate its resources when the Unix process exits. */
    th->p_detached = 1;
    __pthread_unlock(&handle->h_lock);
  }
}

/* Send a signal to all running threads */

static void pthread_kill_all_threads(int sig, int main_thread_also)
{
  pthread_descr th;
  for (th = __pthread_main_thread->p_nextlive;
       th != __pthread_main_thread;
       th = th->p_nextlive) {
    kill(th->p_pid, sig);
  }
  if (main_thread_also) {
    kill(__pthread_main_thread->p_pid, sig);
  }
}

/* Process-wide exit() */

static void pthread_handle_exit(pthread_descr issuing_thread, int exitcode)
{
  pthread_descr th;
  __pthread_exit_requested = 1;
  __pthread_exit_code = exitcode;
  /* Send the CANCEL signal to all running threads, including the main
     thread, but excluding the thread from which the exit request originated
     (that thread must complete the exit, e.g. calling atexit functions
     and flushing stdio buffers). */
  for (th = issuing_thread->p_nextlive;
       th != issuing_thread;
       th = th->p_nextlive) {
    kill(th->p_pid, __pthread_sig_cancel);
  }
  /* Now, wait for all these threads, so that they don't become zombies
     and their times are properly added to the thread manager's times. */
  for (th = issuing_thread->p_nextlive;
       th != issuing_thread;
       th = th->p_nextlive) {
    waitpid(th->p_pid, NULL, __WCLONE);
  }
  restart(issuing_thread);
  _exit(0);
}

/* Handler for __pthread_sig_restart in thread manager thread */

void __pthread_manager_sighandler(int sig)
{
  terminated_children = 1;
}

/* Adjust priority of thread manager so that it always run at a priority
   higher than all threads */

void __pthread_manager_adjust_prio(int thread_prio)
{
  struct sched_param param;

  if (thread_prio <= __pthread_manager_thread.p_priority) return;
  param.sched_priority =
    thread_prio < __sched_get_priority_max(SCHED_FIFO)
    ? thread_prio + 1 : thread_prio;
  __sched_setscheduler(__pthread_manager_thread.p_pid, SCHED_FIFO, &param);
  __pthread_manager_thread.p_priority = thread_prio;
}
