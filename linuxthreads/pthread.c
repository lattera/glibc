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

/* Thread creation, initialization, and basic low-level routines */

#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include "pthread.h"
#include "internals.h"
#include "spinlock.h"
#include "restart.h"

/* Descriptor of the initial thread */

struct _pthread_descr_struct __pthread_initial_thread = {
  &__pthread_initial_thread,  /* pthread_descr p_nextlive */
  &__pthread_initial_thread,  /* pthread_descr p_prevlive */
  NULL,                       /* pthread_descr p_nextwaiting */
  PTHREAD_THREADS_MAX,        /* pthread_t p_tid */
  0,                          /* int p_pid */
  0,                          /* int p_priority */
  &__pthread_handles[0].h_lock, /* struct _pthread_fastlock * p_lock */
  0,                          /* int p_signal */
  NULL,                       /* sigjmp_buf * p_signal_buf */
  NULL,                       /* sigjmp_buf * p_cancel_buf */
  0,                          /* char p_terminated */
  0,                          /* char p_detached */
  0,                          /* char p_exited */
  NULL,                       /* void * p_retval */
  0,                          /* int p_retval */
  NULL,                       /* pthread_descr p_joining */
  NULL,                       /* struct _pthread_cleanup_buffer * p_cleanup */
  0,                          /* char p_cancelstate */
  0,                          /* char p_canceltype */
  0,                          /* char p_canceled */
  NULL,                       /* int *p_errnop */
  0,                          /* int p_errno */
  NULL,                       /* int *p_h_errnop */
  0,                          /* int p_h_errno */
  NULL,                       /* char * p_in_sighandler */
  0,                          /* char p_sigwaiting */
  PTHREAD_START_ARGS_INITIALIZER, /* struct pthread_start_args p_start_args */
  {NULL},                     /* void ** p_specific[PTHREAD_KEY_1STLEVEL_SIZE] */
  {NULL},                     /* void * p_libc_specific[_LIBC_TSD_KEY_N] */
  0,                          /* int p_userstack */
  NULL,                       /* void * p_guardaddr */
  0,                          /* size_t p_guardsize */
  &__pthread_initial_thread,  /* pthread_descr p_self */
  0                           /* Always index 0 */
};

/* Descriptor of the manager thread; none of this is used but the error
   variables, the p_pid and p_priority fields,
   and the address for identification.  */

struct _pthread_descr_struct __pthread_manager_thread = {
  NULL,                       /* pthread_descr p_nextlive */
  NULL,                       /* pthread_descr p_prevlive */
  NULL,                       /* pthread_descr p_nextwaiting */
  0,                          /* int p_tid */
  0,                          /* int p_pid */
  0,                          /* int p_priority */
  NULL,                       /* struct _pthread_fastlock * p_lock */
  0,                          /* int p_signal */
  NULL,                       /* sigjmp_buf * p_signal_buf */
  NULL,                       /* sigjmp_buf * p_cancel_buf */
  0,                          /* char p_terminated */
  0,                          /* char p_detached */
  0,                          /* char p_exited */
  NULL,                       /* void * p_retval */
  0,                          /* int p_retval */
  NULL,                       /* pthread_descr p_joining */
  NULL,                       /* struct _pthread_cleanup_buffer * p_cleanup */
  0,                          /* char p_cancelstate */
  0,                          /* char p_canceltype */
  0,                          /* char p_canceled */
  &__pthread_manager_thread.p_errno, /* int *p_errnop */
  0,                          /* int p_errno */
  NULL,                       /* int *p_h_errnop */
  0,                          /* int p_h_errno */
  NULL,                       /* char * p_in_sighandler */
  0,                          /* char p_sigwaiting */
  PTHREAD_START_ARGS_INITIALIZER, /* struct pthread_start_args p_start_args */
  {NULL},                     /* void ** p_specific[PTHREAD_KEY_1STLEVEL_SIZE] */
  {NULL},                     /* void * p_libc_specific[_LIBC_TSD_KEY_N] */
  0,                          /* int p_userstack */
  NULL,                       /* void * p_guardaddr */
  0,                          /* size_t p_guardsize */
  &__pthread_manager_thread,  /* pthread_descr p_self */
  1                           /* Always index 1 */
};

/* Pointer to the main thread (the father of the thread manager thread) */
/* Originally, this is the initial thread, but this changes after fork() */

pthread_descr __pthread_main_thread = &__pthread_initial_thread;

/* Limit between the stack of the initial thread (above) and the
   stacks of other threads (below). Aligned on a STACK_SIZE boundary. */

char *__pthread_initial_thread_bos = NULL;

/* File descriptor for sending requests to the thread manager. */
/* Initially -1, meaning that the thread manager is not running. */

int __pthread_manager_request = -1;

/* Other end of the pipe for sending requests to the thread manager. */

int __pthread_manager_reader;

/* Limits of the thread manager stack */

char *__pthread_manager_thread_bos = NULL;
char *__pthread_manager_thread_tos = NULL;

/* For process-wide exit() */

int __pthread_exit_requested = 0;
int __pthread_exit_code = 0;

/* Communicate relevant LinuxThreads constants to gdb */

const int __pthread_threads_max = PTHREAD_THREADS_MAX;
const int __pthread_sizeof_handle = sizeof(struct pthread_handle_struct);
const int __pthread_offsetof_descr = offsetof(struct pthread_handle_struct,
                                              h_descr);
const int __pthread_offsetof_pid = offsetof(struct _pthread_descr_struct,
                                            p_pid);

/* Signal numbers used for the communication.  */
#ifdef SIGRTMIN
int __pthread_sig_restart;
int __pthread_sig_cancel;
#else
int __pthread_sig_restart = DEFAULT_SIG_RESTART;
int __pthread_sig_cancel = DEFAULT_SIG_CANCEL;
#endif

/* These variables are used by the setup code.  */
extern int _errno;
extern int _h_errno;

/* Forward declarations */

static void pthread_exit_process(int retcode, void *arg);
#ifndef __i386__
static void pthread_handle_sigcancel(int sig);
static void pthread_handle_sigrestart(int sig);
#else
static void pthread_handle_sigcancel(int sig, struct sigcontext ctx);
static void pthread_handle_sigrestart(int sig, struct sigcontext ctx);
#endif

/* Initialize the pthread library.
   Initialization is split in two functions:
   - a constructor function that blocks the __pthread_sig_restart signal
     (must do this very early, since the program could capture the signal
      mask with e.g. sigsetjmp before creating the first thread);
   - a regular function called from pthread_create when needed. */

static void pthread_initialize(void) __attribute__((constructor));

static void pthread_initialize(void)
{
  struct sigaction sa;
  sigset_t mask;
  struct rlimit limit;
  int max_stack;

  /* If already done (e.g. by a constructor called earlier!), bail out */
  if (__pthread_initial_thread_bos != NULL) return;
#ifdef TEST_FOR_COMPARE_AND_SWAP
  /* Test if compare-and-swap is available */
  __pthread_has_cas = compare_and_swap_is_available();
#endif
  /* For the initial stack, reserve at least STACK_SIZE bytes of stack
     below the current stack address, and align that on a
     STACK_SIZE boundary. */
  __pthread_initial_thread_bos =
    (char *)(((long)CURRENT_STACK_FRAME - 2 * STACK_SIZE) & ~(STACK_SIZE - 1));
  /* Play with the stack size limit to make sure that no stack ever grows
     beyond STACK_SIZE minus two pages (one page for the thread descriptor
     immediately beyond, and one page to act as a guard page). */
  getrlimit(RLIMIT_STACK, &limit);
  max_stack = STACK_SIZE - 2 * __getpagesize();
  if (limit.rlim_cur > max_stack) {
    limit.rlim_cur = max_stack;
    setrlimit(RLIMIT_STACK, &limit);
  }
  /* Update the descriptor for the initial thread. */
  __pthread_initial_thread.p_pid = __getpid();
  /* If we have special thread_self processing, initialize that for the
     main thread now.  */
#ifdef INIT_THREAD_SELF
  INIT_THREAD_SELF(&__pthread_initial_thread, 0);
#endif
  /* The errno/h_errno variable of the main thread are the global ones.  */
  __pthread_initial_thread.p_errnop = &_errno;
  __pthread_initial_thread.p_h_errnop = &_h_errno;
#ifdef SIGRTMIN
  /* Allocate the signals used.  */
  __pthread_sig_restart = __libc_allocate_rtsig (1);
  __pthread_sig_cancel = __libc_allocate_rtsig (1);
  if (__pthread_sig_restart < 0 || __pthread_sig_cancel < 0)
    {
      /* The kernel does not support real-time signals.  Use as before
	 the available signals in the fixed set.  */
      __pthread_sig_restart = DEFAULT_SIG_RESTART;
      __pthread_sig_cancel = DEFAULT_SIG_CANCEL;
    }
#endif
  /* Setup signal handlers for the initial thread.
     Since signal handlers are shared between threads, these settings
     will be inherited by all other threads. */
#ifndef __i386__
  sa.sa_handler = pthread_handle_sigrestart;
#else
  sa.sa_handler = (__sighandler_t) pthread_handle_sigrestart;
#endif
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART; /* does not matter for regular threads, but
                               better for the thread manager */
  __sigaction(__pthread_sig_restart, &sa, NULL);
#ifndef __i386__
  sa.sa_handler = pthread_handle_sigcancel;
#else
  sa.sa_handler = (__sighandler_t) pthread_handle_sigcancel;
#endif
  sa.sa_flags = 0;
  __sigaction(__pthread_sig_cancel, &sa, NULL);

  /* Initially, block __pthread_sig_restart. Will be unblocked on demand. */
  sigemptyset(&mask);
  sigaddset(&mask, __pthread_sig_restart);
  sigprocmask(SIG_BLOCK, &mask, NULL);
  /* Register an exit function to kill all other threads. */
  /* Do it early so that user-registered atexit functions are called
     before pthread_exit_process. */
  __on_exit(pthread_exit_process, NULL);
}

void __pthread_initialize(void)
{
  pthread_initialize();
}

int __pthread_initialize_manager(void)
{
  int manager_pipe[2];
  int pid;
  struct pthread_request request;

  /* If basic initialization not done yet (e.g. we're called from a
     constructor run before our constructor), do it now */
  if (__pthread_initial_thread_bos == NULL) pthread_initialize();
  /* Setup stack for thread manager */
  __pthread_manager_thread_bos = malloc(THREAD_MANAGER_STACK_SIZE);
  if (__pthread_manager_thread_bos == NULL) return -1;
  __pthread_manager_thread_tos =
    __pthread_manager_thread_bos + THREAD_MANAGER_STACK_SIZE;
  /* Setup pipe to communicate with thread manager */
  if (pipe(manager_pipe) == -1) {
    free(__pthread_manager_thread_bos);
    return -1;
  }
  /* Start the thread manager */
  pid = __clone(__pthread_manager, (void **) __pthread_manager_thread_tos,
                CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND
#ifdef CLONE_PTRACE
		| CLONE_PTRACE
#endif
		, (void *)(long)manager_pipe[0]);
  if (pid == -1) {
    free(__pthread_manager_thread_bos);
    __libc_close(manager_pipe[0]);
    __libc_close(manager_pipe[1]);
    return -1;
  }
  __pthread_manager_request = manager_pipe[1]; /* writing end */
  __pthread_manager_reader = manager_pipe[0]; /* reading end */
  __pthread_manager_thread.p_pid = pid;
  /* Make gdb aware of new thread manager */
  if (__pthread_threads_debug) raise(__pthread_sig_cancel);
  /* Synchronize debugging of the thread manager */
  request.req_kind = REQ_DEBUG;
  __libc_write(__pthread_manager_request, (char *) &request, sizeof(request));
  return 0;
}

/* Thread creation */

int __pthread_create_2_1(pthread_t *thread, const pthread_attr_t *attr,
			 void * (*start_routine)(void *), void *arg)
{
  pthread_descr self = thread_self();
  struct pthread_request request;
  if (__pthread_manager_request < 0) {
    if (__pthread_initialize_manager() < 0) return EAGAIN;
  }
  request.req_thread = self;
  request.req_kind = REQ_CREATE;
  request.req_args.create.attr = attr;
  request.req_args.create.fn = start_routine;
  request.req_args.create.arg = arg;
  sigprocmask(SIG_SETMASK, (const sigset_t *) NULL,
              &request.req_args.create.mask);
  __libc_write(__pthread_manager_request, (char *) &request, sizeof(request));
  suspend(self);
  if (THREAD_GETMEM(self, p_retcode) == 0)
    *thread = (pthread_t) THREAD_GETMEM(self, p_retval);
  return THREAD_GETMEM(self, p_retcode);
}

#if defined HAVE_ELF && defined PIC && defined DO_VERSIONING
default_symbol_version (__pthread_create_2_1, pthread_create, GLIBC_2.1);

int __pthread_create_2_0(pthread_t *thread, const pthread_attr_t *attr,
			 void * (*start_routine)(void *), void *arg)
{
  /* The ATTR attribute is not really of type `pthread_attr_t *'.  It has
     the old size and access to the new members might crash the program.
     We convert the struct now.  */
  pthread_attr_t new_attr;

  if (attr != NULL)
    {
      size_t ps = __getpagesize ();

      memcpy (&new_attr, attr,
	      (size_t) &(((pthread_attr_t*)NULL)->__guardsize));
      new_attr.__guardsize = ps;
      new_attr.__stackaddr_set = 0;
      new_attr.__stackaddr = NULL;
      new_attr.__stacksize = STACK_SIZE - ps;
      attr = &new_attr;
    }
  return __pthread_create_2_1 (thread, attr, start_routine, arg);
}
symbol_version (__pthread_create_2_0, pthread_create, GLIBC_2.0);
#else
strong_alias (__pthread_create_2_1, pthread_create)
#endif

/* Simple operations on thread identifiers */

pthread_t pthread_self(void)
{
  pthread_descr self = thread_self();
  return THREAD_GETMEM(self, p_tid);
}

int pthread_equal(pthread_t thread1, pthread_t thread2)
{
  return thread1 == thread2;
}

/* Helper function for thread_self in the case of user-provided stacks */

#ifndef THREAD_SELF

pthread_descr __pthread_find_self()
{
  char * sp = CURRENT_STACK_FRAME;
  pthread_handle h;

  /* __pthread_handles[0] is the initial thread, __pthread_handles[1] is
     the manager threads handled specially in thread_self(), so start at 2 */
  h = __pthread_handles + 2;
  while (! (sp <= (char *) h->h_descr && sp >= h->h_bottom)) h++;
  return h->h_descr;
}

#endif

/* Thread scheduling */

int pthread_setschedparam(pthread_t thread, int policy,
                          const struct sched_param *param)
{
  pthread_handle handle = thread_handle(thread);
  pthread_descr th;

  __pthread_lock(&handle->h_lock, NULL);
  if (invalid_handle(handle, thread)) {
    __pthread_unlock(&handle->h_lock);
    return ESRCH;
  }
  th = handle->h_descr;
  if (__sched_setscheduler(th->p_pid, policy, param) == -1) {
    __pthread_unlock(&handle->h_lock);
    return errno;
  }
  th->p_priority = policy == SCHED_OTHER ? 0 : param->sched_priority;
  __pthread_unlock(&handle->h_lock);
  if (__pthread_manager_request >= 0)
    __pthread_manager_adjust_prio(th->p_priority);
  return 0;
}

int pthread_getschedparam(pthread_t thread, int *policy,
                          struct sched_param *param)
{
  pthread_handle handle = thread_handle(thread);
  int pid, pol;

  __pthread_lock(&handle->h_lock, NULL);
  if (invalid_handle(handle, thread)) {
    __pthread_unlock(&handle->h_lock);
    return ESRCH;
  }
  pid = handle->h_descr->p_pid;
  __pthread_unlock(&handle->h_lock);
  pol = __sched_getscheduler(pid);
  if (pol == -1) return errno;
  if (__sched_getparam(pid, param) == -1) return errno;
  *policy = pol;
  return 0;
}

/* Process-wide exit() request */

static void pthread_exit_process(int retcode, void *arg)
{
  struct pthread_request request;
  pthread_descr self = thread_self();

  if (__pthread_manager_request >= 0) {
    request.req_thread = self;
    request.req_kind = REQ_PROCESS_EXIT;
    request.req_args.exit.code = retcode;
    __libc_write(__pthread_manager_request,
		 (char *) &request, sizeof(request));
    suspend(self);
    /* Main thread should accumulate times for thread manager and its
       children, so that timings for main thread account for all threads. */
    if (self == __pthread_main_thread)
      waitpid(__pthread_manager_thread.p_pid, NULL, __WCLONE);
  }
}

/* The handler for the RESTART signal just records the signal received
   in the thread descriptor, and optionally performs a siglongjmp
   (for pthread_cond_timedwait). */

#ifndef __i386__
static void pthread_handle_sigrestart(int sig)
{
  pthread_descr self = thread_self();
#else
static void pthread_handle_sigrestart(int sig, struct sigcontext ctx)
{
  pthread_descr self;
  asm volatile ("movw %w0,%%gs" : : "r" (ctx.gs));
  self = thread_self();
#endif
  THREAD_SETMEM(self, p_signal, sig); 
  if (THREAD_GETMEM(self, p_signal_jmp) != NULL) 
    siglongjmp(*THREAD_GETMEM(self, p_signal_jmp), 1);
}

/* The handler for the CANCEL signal checks for cancellation
   (in asynchronous mode), for process-wide exit and exec requests.
   For the thread manager thread, redirect the signal to 
   __pthread_manager_sighandler.
   The debugging strategy is as follows:
   On reception of a REQ_DEBUG request (sent by new threads created to
   the thread manager under debugging mode), the thread manager throws
   __pthread_sig_cancel to itself. The debugger (if active) intercepts
   this signal, takes into account new threads and continue execution
   of the thread manager by propagating the signal because it doesn't
   know what it is specifically done for. In the current implementation,
   the thread manager simply discards it. */

#ifndef __i386__
static void pthread_handle_sigcancel(int sig)
{
  pthread_descr self = thread_self();
  sigjmp_buf * jmpbuf;
#else
static void pthread_handle_sigcancel(int sig, struct sigcontext ctx)
{
  pthread_descr self;
  sigjmp_buf * jmpbuf;
  asm volatile ("movw %w0,%%gs" : : "r" (ctx.gs));
  self = thread_self();
#endif

  if (self == &__pthread_manager_thread)
    {
      __pthread_manager_sighandler(sig);
      return;
    }
  if (__pthread_exit_requested) {
    /* Main thread should accumulate times for thread manager and its
       children, so that timings for main thread account for all threads. */
    if (self == __pthread_main_thread)
      waitpid(__pthread_manager_thread.p_pid, NULL, __WCLONE);
    _exit(__pthread_exit_code);
  }
  if (THREAD_GETMEM(self, p_canceled)
      && THREAD_GETMEM(self, p_cancelstate) == PTHREAD_CANCEL_ENABLE) {
    if (THREAD_GETMEM(self, p_canceltype) == PTHREAD_CANCEL_ASYNCHRONOUS)
      pthread_exit(PTHREAD_CANCELED);
    jmpbuf = THREAD_GETMEM(self, p_cancel_jmp);
    if (jmpbuf != NULL) {
      THREAD_SETMEM(self, p_cancel_jmp, NULL);
      siglongjmp(*jmpbuf, 1);
    }
  }
}

/* Reset the state of the thread machinery after a fork().
   Close the pipe used for requests and set the main thread to the forked
   thread.
   Notice that we can't free the stack segments, as the forked thread
   may hold pointers into them. */

void __pthread_reset_main_thread()
{
  pthread_descr self = thread_self();

  if (__pthread_manager_request != -1) {
    /* Free the thread manager stack */
    free(__pthread_manager_thread_bos);
    __pthread_manager_thread_bos = __pthread_manager_thread_tos = NULL;
    /* Close the two ends of the pipe */
    __libc_close(__pthread_manager_request);
    __libc_close(__pthread_manager_reader);
    __pthread_manager_request = __pthread_manager_reader = -1;
  }
  /* Update the pid of the main thread */
  THREAD_SETMEM(self, p_pid, __getpid());
  /* Make the forked thread the main thread */
  __pthread_main_thread = self;
  THREAD_SETMEM(self, p_nextlive, self);
  THREAD_SETMEM(self, p_prevlive, self);
  /* Now this thread modifies the global variables.  */
  THREAD_SETMEM(self, p_errnop, &_errno);
  THREAD_SETMEM(self, p_h_errnop, &_h_errno);
}

/* Process-wide exec() request */

void __pthread_kill_other_threads_np(void)
{
  /* Terminate all other threads and thread manager */
  pthread_exit_process(0, NULL);
  /* Make current thread the main thread in case the calling thread
     changes its mind, does not exec(), and creates new threads instead. */
  __pthread_reset_main_thread();
}
weak_alias (__pthread_kill_other_threads_np, pthread_kill_other_threads_np)

/* Concurrency symbol level.  */
static int current_level;

int __pthread_setconcurrency(int level)
{
  /* We don't do anything unless we have found a useful interpretation.  */
  current_level = level;
  return 0;
}
weak_alias (__pthread_setconcurrency, pthread_setconcurrency)

int __pthread_getconcurrency(void)
{
  return current_level;
}
weak_alias (__pthread_getconcurrency, pthread_getconcurrency)

/* Debugging aid */

#ifdef DEBUG
#include <stdarg.h>

void __pthread_message(char * fmt, ...)
{
  char buffer[1024];
  va_list args;
  sprintf(buffer, "%05d : ", __getpid());
  va_start(args, fmt);
  vsnprintf(buffer + 8, sizeof(buffer) - 8, fmt, args);
  va_end(args);
  __libc_write(2, buffer, strlen(buffer));
}

#endif


#ifndef PIC
/* We need a hook to force the cancelation wrappers to be linked in when
   static libpthread is used.  */
extern const int __pthread_provide_wrappers;
static const int *const __pthread_require_wrappers =
  &__pthread_provide_wrappers;
#endif
