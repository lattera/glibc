/* Handle general operations.
   Copyright (C) 1997,1998,1999,2000,2001,2002,2003,2006
   Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1997.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <kaio_misc.h>

#ifndef USE_KAIO
#include <aio_misc.c>
#else

#include <aio.h>
#include <assert.h>
#include <atomic.h>
#include <errno.h>
#include <limits.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/sysmacros.h>

#ifndef aio_create_helper_thread
# define aio_create_helper_thread __aio_create_helper_thread

extern inline int
__aio_create_helper_thread (pthread_t *threadp, void *(*tf) (void *), void *arg)
{
  pthread_attr_t attr;

  /* Make sure the thread is created detached.  */
  pthread_attr_init (&attr);
  pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);

  int ret = pthread_create (threadp, &attr, tf, arg);

  (void) pthread_attr_destroy (&attr);
  return ret;
}

#endif


static void add_request_to_runlist (struct requestlist *newrequest)
	internal_function;
static int add_request_to_list (struct requestlist *newrequest, int fildes,
				int prio)
	internal_function;
static void * handle_kernel_aio (void *arg);
static void kernel_callback (kctx_t ctx, struct kiocb *kiocb, long res,
			     long res2);

/* Pool of request list entries.  */
static struct requestlist **pool;

/* Number of total and allocated pool entries.  */
static size_t pool_max_size;
static size_t pool_size;

/* Kernel AIO context.  */
kctx_t __aio_kioctx = KCTX_NONE;
int __have_no_kernel_aio;
int __kernel_thread_started;

/* We implement a two dimensional array but allocate each row separately.
   The macro below determines how many entries should be used per row.
   It should better be a power of two.  */
#define ENTRIES_PER_ROW	32

/* How many rows we allocate at once.  */
#define ROWS_STEP	8

/* List of available entries.  */
static struct requestlist *freelist;

/* List of request waiting to be processed.  */
static struct requestlist *runlist;

/* Structure list of all currently processed requests.  */
static struct requestlist *requests, *krequests;

/* Number of threads currently running.  */
static int nthreads;

/* Number of threads waiting for work to arrive. */
static int idle_thread_count;


/* These are the values used to optimize the use of AIO.  The user can
   overwrite them by using the `aio_init' function.  */
static struct aioinit optim =
{
  20,	/* int aio_threads;	Maximal number of threads.  */
  64,	/* int aio_num;		Number of expected simultanious requests. */
  0,
  0,
  0,
  0,
  1,
  0
};


/* Since the list is global we need a mutex protecting it.  */
pthread_mutex_t __aio_requests_mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

/* When you add a request to the list and there are idle threads present,
   you signal this condition variable. When a thread finishes work, it waits
   on this condition variable for a time before it actually exits. */
pthread_cond_t __aio_new_request_notification = PTHREAD_COND_INITIALIZER;


/* Functions to handle request list pool.  */
static struct requestlist *
get_elem (void)
{
  struct requestlist *result;

  if (freelist == NULL)
    {
      struct requestlist *new_row;
      int cnt;

      assert (sizeof (struct aiocb) == sizeof (struct aiocb64));

      if (pool_size + 1 >= pool_max_size)
	{
	  size_t new_max_size = pool_max_size + ROWS_STEP;
	  struct requestlist **new_tab;

	  new_tab = (struct requestlist **)
	    realloc (pool, new_max_size * sizeof (struct requestlist *));

	  if (new_tab == NULL)
	    return NULL;

	  pool_max_size = new_max_size;
	  pool = new_tab;
	}

      /* Allocate the new row.  */
      cnt = pool_size == 0 ? optim.aio_num : ENTRIES_PER_ROW;
      new_row = (struct requestlist *) calloc (cnt,
					       sizeof (struct requestlist));
      if (new_row == NULL)
	return NULL;

      pool[pool_size++] = new_row;

      /* Put all the new entries in the freelist.  */
      do
	{
	  new_row->next_prio = freelist;
	  freelist = new_row++;
	}
      while (--cnt > 0);
    }

  result = freelist;
  freelist = freelist->next_prio;

  return result;
}


void
internal_function
__aio_free_request (struct requestlist *elem)
{
  elem->running = no;
  elem->next_prio = freelist;
  freelist = elem;
}


struct requestlist *
internal_function
__aio_find_req (aiocb_union *elem)
{
  struct requestlist *runp;
  int fildes = elem->aiocb.aio_fildes;
  int i;

  for (i = 0; i < 2; i++)
    {
      runp = i ? requests : krequests;

      while (runp != NULL && runp->aiocbp->aiocb.aio_fildes < fildes)
	runp = runp->next_fd;

      if (runp != NULL)
	{
	  if (runp->aiocbp->aiocb.aio_fildes != fildes)
	    runp = NULL;
	  else
	    while (runp != NULL && runp->aiocbp != elem)
	      runp = runp->next_prio;
	  if (runp != NULL)
	    return runp;
	}
    }

  return NULL;
}


struct requestlist *
internal_function
__aio_find_req_fd (int fildes)
{
  struct requestlist *runp = requests;

  while (runp != NULL && runp->aiocbp->aiocb.aio_fildes < fildes)
    runp = runp->next_fd;

  return (runp != NULL && runp->aiocbp->aiocb.aio_fildes == fildes
	  ? runp : NULL);
}


struct requestlist *
internal_function
__aio_find_kreq_fd (int fildes)
{
  struct requestlist *runp = krequests;

  while (runp != NULL && runp->aiocbp->aiocb.aio_fildes < fildes)
    runp = runp->next_fd;

  return (runp != NULL && runp->aiocbp->aiocb.aio_fildes == fildes
	  ? runp : NULL);
}


void
internal_function
__aio_remove_request (struct requestlist *last, struct requestlist *req,
		      int all)
{
  assert (req->running == yes || req->running == queued
	  || req->running == done);
  assert (req->kioctx == KCTX_NONE);

  if (last != NULL)
    last->next_prio = all ? NULL : req->next_prio;
  else
    {
      if (all || req->next_prio == NULL)
	{
	  if (req->last_fd != NULL)
	    req->last_fd->next_fd = req->next_fd;
	  else
	    requests = req->next_fd;
	  if (req->next_fd != NULL)
	    req->next_fd->last_fd = req->last_fd;
	}
      else
	{
	  if (req->last_fd != NULL)
	    req->last_fd->next_fd = req->next_prio;
	  else
	    requests = req->next_prio;

	  if (req->next_fd != NULL)
	    req->next_fd->last_fd = req->next_prio;

	  req->next_prio->last_fd = req->last_fd;
	  req->next_prio->next_fd = req->next_fd;

	  /* Mark this entry as runnable.  */
	  req->next_prio->running = yes;
	}

      if (req->running == yes)
	{
	  struct requestlist *runp = runlist;

	  last = NULL;
	  while (runp != NULL)
	    {
	      if (runp == req)
		{
		  if (last == NULL)
		    runlist = runp->next_run;
		  else
		    last->next_run = runp->next_run;
		  break;
		}
	      last = runp;
	      runp = runp->next_run;
	    }
	}
    }
}

void
internal_function
__aio_remove_krequest (struct requestlist *req)
{
  assert (req->running == yes || req->running == queued
	  || req->running == done);
  assert (req->kioctx != KCTX_NONE);

  if (req->prev_prio != NULL)
    {
      req->prev_prio->next_prio = req->next_prio;
      if (req->next_prio != NULL)
	req->next_prio->prev_prio = req->prev_prio;
    }
  else if (req->next_prio == NULL)
    {
      if (req->last_fd != NULL)
	req->last_fd->next_fd = req->next_fd;
      else
	krequests = req->next_fd;
      if (req->next_fd != NULL)
	req->next_fd->last_fd = req->last_fd;
    }
  else
    {
      if (req->last_fd != NULL)
	req->last_fd->next_fd = req->next_prio;
      else
	krequests = req->next_prio;
      if (req->next_fd != NULL)
	req->next_fd->last_fd = req->next_prio;

      req->next_prio->prev_prio = NULL;
      req->next_prio->last_fd = req->last_fd;
      req->next_prio->next_fd = req->next_fd;
    }
}


/* The thread handler.  */
static void *handle_fildes_io (void *arg);
static int wait_for_kernel_requests (int fildes);


/* User optimization.  */
void
__aio_init (const struct aioinit *init)
{
  /* Get the mutex.  */
  pthread_mutex_lock (&__aio_requests_mutex);

  /* Only allow writing new values if the table is not yet allocated.  */
  if (pool == NULL)
    {
      optim.aio_threads = init->aio_threads < 1 ? 1 : init->aio_threads;
      optim.aio_num = (init->aio_num < ENTRIES_PER_ROW
		       ? ENTRIES_PER_ROW
		       : init->aio_num & ~ENTRIES_PER_ROW);
    }

  if (init->aio_idle_time != 0)
    optim.aio_idle_time = init->aio_idle_time;

  /* Release the mutex.  */
  pthread_mutex_unlock (&__aio_requests_mutex);
}
weak_alias (__aio_init, aio_init)

static void
kernel_callback (kctx_t ctx, struct kiocb *kiocb, long res, long res2)
{
  struct requestlist *req = (struct requestlist *)kiocb;
  long errcode = 0;

  if (res < 0 && res > -1000)
    {
      errcode = -res;
      res = -1;
    }
  req->aiocbp->aiocb.__return_value = res;
  atomic_write_barrier ();
  req->aiocbp->aiocb.__error_code = errcode;
  __aio_notify (req);
  assert (req->running == allocated);
  req->running = done;
  __aio_remove_krequest (req);
  __aio_free_request (req);
}

void
internal_function
__aio_read_one_event (void)
{
  struct kio_event ev[10];
  struct timespec ts;
  int count, i;

  if (__aio_kioctx == KCTX_NONE)
    return;
  ts.tv_sec = 0;
  ts.tv_nsec = 0;
  do
    {
      INTERNAL_SYSCALL_DECL (err);
      count = INTERNAL_SYSCALL (io_getevents, err, 5, __aio_kioctx, 0, 10,
				ev, &ts);
      if (INTERNAL_SYSCALL_ERROR_P (count, err) || count == 0)
	break;
      pthread_mutex_lock (&__aio_requests_mutex);
      for (i = 0; i < count; i++)
	{
	  void (*cb)(kctx_t, struct kiocb *, long, long);

	  cb = (void *) (uintptr_t) ev[i].kioe_data;
	  cb (__aio_kioctx, (struct kiocb *) (uintptr_t) ev[i].kioe_obj,
	      ev[i].kioe_res, ev[i].kioe_res2);
	}
      pthread_mutex_unlock (&__aio_requests_mutex);
    }
  while (count == 10);
}

int
internal_function
__aio_wait_for_events (kctx_t kctx, const struct timespec *timespec)
{
  int ret, i;
  struct kio_event ev[10];
  struct timespec ts;
  INTERNAL_SYSCALL_DECL (err);

  pthread_mutex_unlock (&__aio_requests_mutex);
  ts.tv_sec = 0;
  ts.tv_nsec = 0;
  do
    {
      ret = INTERNAL_SYSCALL (io_getevents, err, 5, kctx, 1, 10, ev,
			      timespec);
      if (INTERNAL_SYSCALL_ERROR_P (ret, err) || ret == 0)
	break;

      pthread_mutex_lock (&__aio_requests_mutex);
      for (i = 0; i < ret; i++)
	{
	  void (*cb)(kctx_t, struct kiocb *, long, long);

	  cb = (void *) (uintptr_t) ev[i].kioe_data;
	  cb (kctx, (struct kiocb *) (uintptr_t) ev[i].kioe_obj,
	      ev[i].kioe_res, ev[i].kioe_res2);
	}
      if (ret < 10)
	return 0;
      pthread_mutex_unlock (&__aio_requests_mutex);
      timespec = &ts;
    }
  while (1);

  pthread_mutex_lock (&__aio_requests_mutex);
  return (timespec != &ts
	  && INTERNAL_SYSCALL_ERROR_P (ret, err)
	  && INTERNAL_SYSCALL_ERRNO (ret, err) == ETIMEDOUT) ? ETIMEDOUT : 0;
}

int
internal_function
__aio_create_kernel_thread (void)
{
  pthread_t thid;

  if (__kernel_thread_started)
    return 0;

  if (aio_create_helper_thread (&thid, handle_kernel_aio, NULL) != 0)
    return -1;
  __kernel_thread_started = 1;
  return 0;
}

static void *
handle_kernel_aio (void *arg __attribute__((unused)))
{
  int ret, i;
  INTERNAL_SYSCALL_DECL (err);
  struct kio_event ev[10];

  for (;;)
    {
      ret = INTERNAL_SYSCALL (io_getevents, err, 5, __aio_kioctx, 1, 10, ev,
			      NULL);
      if (INTERNAL_SYSCALL_ERROR_P (ret, err) || ret == 0)
        continue;
      pthread_mutex_lock (&__aio_requests_mutex);
      for (i = 0; i < ret; i++)
	{
	  void (*cb)(kctx_t, struct kiocb *, long, long);

	  cb = (void *) (uintptr_t) ev[i].kioe_data;
	  cb (__aio_kioctx, (struct kiocb *) (uintptr_t) ev[i].kioe_obj,
	      ev[i].kioe_res, ev[i].kioe_res2);
	}
      pthread_mutex_unlock (&__aio_requests_mutex);
    }
  return NULL;
}

static int
internal_function
add_request_to_list (struct requestlist *newp, int fildes, int prio)
{
  struct requestlist *last, *runp, *reqs;

  last = NULL;
  reqs = newp->kioctx != KCTX_NONE ? krequests : requests;
  runp = reqs;

  /* First look whether the current file descriptor is currently
     worked with.  */
  while (runp != NULL
	 && runp->aiocbp->aiocb.aio_fildes < fildes)
    {
      last = runp;
      runp = runp->next_fd;
    }

  if (runp != NULL
      && runp->aiocbp->aiocb.aio_fildes == fildes)
    {
      /* The current file descriptor is worked on.  It makes no sense
	 to start another thread since this new thread would fight
	 with the running thread for the resources.  But we also cannot
	 say that the thread processing this desriptor shall immediately
	 after finishing the current job process this request if there
	 are other threads in the running queue which have a higher
	 priority.  */

      /* Simply enqueue it after the running one according to the
	 priority.  */
      while (runp->next_prio != NULL
	     && runp->next_prio->aiocbp->aiocb.__abs_prio >= prio)
	runp = runp->next_prio;

      newp->next_prio = runp->next_prio;
      runp->next_prio = newp;
      if (newp->kioctx != KCTX_NONE)
	{
	  newp->prev_prio = runp;
	  if (newp->next_prio != NULL)
	    newp->next_prio->prev_prio = newp;
	}
      return queued;
    }
  else
    {
      /* Enqueue this request for a new descriptor.  */
      if (last == NULL)
	{
	  newp->last_fd = NULL;
	  newp->next_fd = reqs;
	  if (reqs != NULL)
	    reqs->last_fd = newp;
	  if (newp->kioctx != KCTX_NONE)
	    krequests = newp;
	  else
	    requests = newp;
	}
      else
	{
	  newp->next_fd = last->next_fd;
	  newp->last_fd = last;
	  last->next_fd = newp;
	  if (newp->next_fd != NULL)
	    newp->next_fd->last_fd = newp;
	}

      newp->next_prio = NULL;
      if (newp->kioctx != KCTX_NONE)
	newp->prev_prio = NULL;
      return yes;
    }
}

static int
internal_function
__aio_enqueue_user_request (struct requestlist *newp)
{
  int result = 0;
  int running = add_request_to_list (newp, newp->aiocbp->aiocb.aio_fildes,
				     newp->aiocbp->aiocb.__abs_prio);

  if (running == yes)
    {
      /* We try to create a new thread for this file descriptor.  The
	 function which gets called will handle all available requests
	 for this descriptor and when all are processed it will
	 terminate.

	 If no new thread can be created or if the specified limit of
	 threads for AIO is reached we queue the request.  */

      /* See if we need to and are able to create a thread.  */
      if (nthreads < optim.aio_threads && idle_thread_count == 0)
	{
	  pthread_t thid;

	  running = newp->running = allocated;

	  /* Now try to start a thread.  */
	  if (aio_create_helper_thread (&thid, handle_fildes_io, newp) == 0)
	    /* We managed to enqueue the request.  All errors which can
	       happen now can be recognized by calls to `aio_return' and
	       `aio_error'.  */
	    ++nthreads;
	  else
	    {
	      /* Reset the running flag.  The new request is not running.  */
	      running = newp->running = yes;

	      if (nthreads == 0)
		/* We cannot create a thread in the moment and there is
		   also no thread running.  This is a problem.  `errno' is
		   set to EAGAIN if this is only a temporary problem.  */
		result = -1;
	    }
	}
    }

  /* Enqueue the request in the run queue if it is not yet running.  */
  if (running == yes && result == 0)
    {
      add_request_to_runlist (newp);

      /* If there is a thread waiting for work, then let it know that we
	 have just given it something to do. */
      if (idle_thread_count > 0)
	pthread_cond_signal (&__aio_new_request_notification);
    }

  if (result == 0)
    newp->running = running;
  return result;
}

/* The main function of the async I/O handling.  It enqueues requests
   and if necessary starts and handles threads.  */
struct requestlist *
internal_function
__aio_enqueue_request_ctx (aiocb_union *aiocbp, int operation, kctx_t kctx)
{
  int policy, prio;
  struct sched_param param;
  struct requestlist *newp;
  int op = (operation & 0xffff);

  if (op == LIO_SYNC || op == LIO_DSYNC)
    {
      aiocbp->aiocb.aio_reqprio = 0;
      /* FIXME: Kernel doesn't support sync yet.  */
      operation &= ~LIO_KTHREAD;
      kctx = KCTX_NONE;
    }
  else if (aiocbp->aiocb.aio_reqprio < 0
	   || aiocbp->aiocb.aio_reqprio > AIO_PRIO_DELTA_MAX)
    {
      /* Invalid priority value.  */
      __set_errno (EINVAL);
      aiocbp->aiocb.__error_code = EINVAL;
      aiocbp->aiocb.__return_value = -1;
      return NULL;
    }

  if ((operation & LIO_KTHREAD) || kctx != KCTX_NONE)
    {
      /* io_* is only really asynchronous for O_DIRECT or /dev/raw*.  */
      int fl = __fcntl (aiocbp->aiocb.aio_fildes, F_GETFL);
      if (fl < 0 || (fl & O_DIRECT) == 0)
	{
	  struct stat64 st;
	  if (__fxstat64 (_STAT_VER, aiocbp->aiocb.aio_fildes, &st) < 0
	      || ! S_ISCHR (st.st_mode)
	      || major (st.st_rdev) != 162)
	    {
	      operation &= ~LIO_KTHREAD;
	      kctx = KCTX_NONE;
	    }
	}
    }

  /* Compute priority for this request.  */
  pthread_getschedparam (pthread_self (), &policy, &param);
  prio = param.sched_priority - aiocbp->aiocb.aio_reqprio;

  /* Get the mutex.  */
  pthread_mutex_lock (&__aio_requests_mutex);

  if (operation & LIO_KTHREAD)
    {
      if (__aio_kioctx == KCTX_NONE && !__have_no_kernel_aio)
	{
	  int res;
	  INTERNAL_SYSCALL_DECL (err);

	  __aio_kioctx = 0;
	  do
	    res = INTERNAL_SYSCALL (io_setup, err, 2, 1024, &__aio_kioctx);
	  while (INTERNAL_SYSCALL_ERROR_P (res, err)
		 && INTERNAL_SYSCALL_ERRNO (res, err) == EINTR);
	  if (INTERNAL_SYSCALL_ERROR_P (res, err))
	    {
              __have_no_kernel_aio = 1;
	      __aio_kioctx = KCTX_NONE;
	    }
	}

      kctx = __aio_kioctx;

      if (kctx != KCTX_NONE && !__kernel_thread_started
	  && ((operation & LIO_KTHREAD_REQUIRED)
	      || aiocbp->aiocb.aio_sigevent.sigev_notify != SIGEV_NONE))
	{
	  if (__aio_create_kernel_thread () < 0)
	    kctx = KCTX_NONE;
	}
    }

  /* Get a new element for the waiting list.  */
  newp = get_elem ();
  if (newp == NULL)
    {
      pthread_mutex_unlock (&__aio_requests_mutex);
      __set_errno (EAGAIN);
      return NULL;
    }
  newp->aiocbp = aiocbp;
#ifdef BROKEN_THREAD_SIGNALS
  newp->caller_pid = (aiocbp->aiocb.aio_sigevent.sigev_notify == SIGEV_SIGNAL
		      ? getpid () : 0);
#endif
  newp->waiting = NULL;
  newp->kioctx = kctx;

  aiocbp->aiocb.__abs_prio = prio;
  aiocbp->aiocb.__policy = policy;
  aiocbp->aiocb.aio_lio_opcode = op;
  aiocbp->aiocb.__error_code = EINPROGRESS;
  aiocbp->aiocb.__return_value = 0;

  if (newp->kioctx != KCTX_NONE)
    {
      int res;
      INTERNAL_SYSCALL_DECL (err);

      aiocb_union *aiocbp = newp->aiocbp;
      struct kiocb *kiocbs[] __attribute__((unused)) = { &newp->kiocb };

      newp->kiocb.kiocb_data = (uintptr_t) kernel_callback;
      switch (op & 127)
        {
        case LIO_READ: newp->kiocb.kiocb_lio_opcode = IO_CMD_PREAD; break;
        case LIO_WRITE: newp->kiocb.kiocb_lio_opcode = IO_CMD_PWRITE; break;
        case LIO_SYNC:
        case LIO_DSYNC: newp->kiocb.kiocb_lio_opcode = IO_CMD_FSYNC; break;
        }
      if (op & 128)
	newp->kiocb.kiocb_offset = aiocbp->aiocb64.aio_offset;
      else
	newp->kiocb.kiocb_offset = aiocbp->aiocb.aio_offset;
      newp->kiocb.kiocb_fildes = aiocbp->aiocb.aio_fildes;
      newp->kiocb.kiocb_buf = (uintptr_t) aiocbp->aiocb.aio_buf;
      newp->kiocb.kiocb_nbytes = aiocbp->aiocb.aio_nbytes;
      /* FIXME.  */
      newp->kiocb.kiocb_req_prio = 0;
      res = INTERNAL_SYSCALL (io_submit, err, 3, newp->kioctx, 1, kiocbs);
      if (! INTERNAL_SYSCALL_ERROR_P (res, err))
	{
	  newp->running = allocated;
	  add_request_to_list (newp, aiocbp->aiocb.aio_fildes, prio);
	  /* Release the mutex.  */
	  pthread_mutex_unlock (&__aio_requests_mutex);
	  return newp;
	}
      newp->kioctx = KCTX_NONE;
    }

  if (__aio_enqueue_user_request (newp))
    {
      /* Something went wrong.  */
      __aio_free_request (newp);
      newp = NULL;
    }

  /* Release the mutex.  */
  pthread_mutex_unlock (&__aio_requests_mutex);

  return newp;
}


static int
wait_for_kernel_requests (int fildes)
{
  pthread_mutex_lock (&__aio_requests_mutex);

  struct requestlist *kreq = __aio_find_kreq_fd (fildes), *req;
  int nent = 0;
  int ret = 0;

  req = kreq;
  while (req)
    {
      if (req->running == allocated)
	++nent;
      req = req->next_prio;
    }

  if (nent)
    {
      if (__aio_create_kernel_thread () < 0)
	{
	  pthread_mutex_unlock (&__aio_requests_mutex);
	  return -1;
	}

#ifndef DONT_NEED_AIO_MISC_COND
      pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
#endif
      struct waitlist waitlist[nent];
      int cnt = 0;

      while (kreq)
	{
	  if (kreq->running == allocated)
	    {
#ifndef DONT_NEED_AIO_MISC_COND
	      waitlist[cnt].cond = &cond;
#endif
	      waitlist[cnt].result = NULL;
	      waitlist[cnt].next = kreq->waiting;
	      waitlist[cnt].counterp = &nent;
	      waitlist[cnt].sigevp = NULL;
#ifdef BROKEN_THREAD_SIGNALS
	      waitlist[cnt].caller_pid = 0;   /* Not needed.  */
#endif
	      kreq->waiting = &waitlist[cnt++];
	    }
	  kreq = kreq->next_prio;
	}

#ifdef DONT_NEED_AIO_MISC_COND
      AIO_MISC_WAIT (ret, nent, NULL, 0);
#else
      do
	pthread_cond_wait (&cond, &__aio_requests_mutex);
      while (nent);

      pthread_cond_destroy (&cond);
#endif
    }

  pthread_mutex_unlock (&__aio_requests_mutex);
  return ret;
}


static void *
handle_fildes_io (void *arg)
{
  pthread_t self = pthread_self ();
  struct sched_param param;
  struct requestlist *runp = (struct requestlist *) arg;
  aiocb_union *aiocbp;
  int policy;
  int fildes;

  pthread_getschedparam (self, &policy, &param);

  do
    {
      /* If runp is NULL, then we were created to service the work queue
	 in general, not to handle any particular request. In that case we
	 skip the "do work" stuff on the first pass, and go directly to the
	 "get work off the work queue" part of this loop, which is near the
	 end. */
      if (runp == NULL)
	pthread_mutex_lock (&__aio_requests_mutex);
      else
	{
	  /* Hopefully this request is marked as running.  */
	  assert (runp->running == allocated);

	  /* Update our variables.  */
	  aiocbp = runp->aiocbp;
	  fildes = aiocbp->aiocb.aio_fildes;

	  /* Change the priority to the requested value (if necessary).  */
	  if (aiocbp->aiocb.__abs_prio != param.sched_priority
	      || aiocbp->aiocb.__policy != policy)
	    {
	      param.sched_priority = aiocbp->aiocb.__abs_prio;
	      policy = aiocbp->aiocb.__policy;
	      pthread_setschedparam (self, policy, &param);
	    }

	  /* Process request pointed to by RUNP.  We must not be disturbed
	     by signals.  */
	  if ((aiocbp->aiocb.aio_lio_opcode & 127) == LIO_READ)
	    {
	      if (aiocbp->aiocb.aio_lio_opcode & 128)
		aiocbp->aiocb.__return_value =
		  TEMP_FAILURE_RETRY (__pread64 (fildes, (void *)
						 aiocbp->aiocb64.aio_buf,
						 aiocbp->aiocb64.aio_nbytes,
						 aiocbp->aiocb64.aio_offset));
	      else
		aiocbp->aiocb.__return_value =
		  TEMP_FAILURE_RETRY (pread (fildes,
					     (void *) aiocbp->aiocb.aio_buf,
					     aiocbp->aiocb.aio_nbytes,
					     aiocbp->aiocb.aio_offset));

	      if (aiocbp->aiocb.__return_value == -1 && errno == ESPIPE)
		/* The Linux kernel is different from others.  It returns
		   ESPIPE if using pread on a socket.  Other platforms
		   simply ignore the offset parameter and behave like
		   read.  */
		aiocbp->aiocb.__return_value =
		  TEMP_FAILURE_RETRY (read (fildes,
					    (void *) aiocbp->aiocb64.aio_buf,
					    aiocbp->aiocb64.aio_nbytes));
	    }
	  else if ((aiocbp->aiocb.aio_lio_opcode & 127) == LIO_WRITE)
	    {
	      if (aiocbp->aiocb.aio_lio_opcode & 128)
		aiocbp->aiocb.__return_value =
		  TEMP_FAILURE_RETRY (__pwrite64 (fildes, (const void *)
						  aiocbp->aiocb64.aio_buf,
						  aiocbp->aiocb64.aio_nbytes,
						  aiocbp->aiocb64.aio_offset));
	      else
		aiocbp->aiocb.__return_value =
		  TEMP_FAILURE_RETRY (__libc_pwrite (fildes, (const void *)
						     aiocbp->aiocb.aio_buf,
						     aiocbp->aiocb.aio_nbytes,
						     aiocbp->aiocb.aio_offset));

	      if (aiocbp->aiocb.__return_value == -1 && errno == ESPIPE)
		/* The Linux kernel is different from others.  It returns
		   ESPIPE if using pwrite on a socket.  Other platforms
		   simply ignore the offset parameter and behave like
		   write.  */
		aiocbp->aiocb.__return_value =
		  TEMP_FAILURE_RETRY (write (fildes,
					     (void *) aiocbp->aiocb64.aio_buf,
					     aiocbp->aiocb64.aio_nbytes));
	    }
	  else if (aiocbp->aiocb.aio_lio_opcode == LIO_DSYNC
		   || aiocbp->aiocb.aio_lio_opcode == LIO_SYNC)
	    {
	      if (wait_for_kernel_requests (fildes) < 0)
		{
		  aiocbp->aiocb.__return_value = -1;
		  __set_errno (ENOMEM);
		}
	      else if (aiocbp->aiocb.aio_lio_opcode == LIO_DSYNC)
		aiocbp->aiocb.__return_value =
		  TEMP_FAILURE_RETRY (fdatasync (fildes));
	      else
		aiocbp->aiocb.__return_value =
		  TEMP_FAILURE_RETRY (fsync (fildes));
	    }
	  else
	    {
	      /* This is an invalid opcode.  */
	      aiocbp->aiocb.__return_value = -1;
	      __set_errno (EINVAL);
	    }

	  /* Get the mutex.  */
	  pthread_mutex_lock (&__aio_requests_mutex);

	  /* In theory we would need here a write memory barrier since the
	     callers test using aio_error() whether the request finished
	     and once this value != EINPROGRESS the field __return_value
	     must be committed to memory.

	     But since the pthread_mutex_lock call involves write memory
	     barriers as well it is not necessary.  */

	  if (aiocbp->aiocb.__return_value == -1)
	    aiocbp->aiocb.__error_code = errno;
	  else
	    aiocbp->aiocb.__error_code = 0;

	  /* Send the signal to notify about finished processing of the
	     request.  */
	  __aio_notify (runp);

	  /* For debugging purposes we reset the running flag of the
	     finished request.  */
	  assert (runp->running == allocated);
	  runp->running = done;

	  /* Now dequeue the current request.  */
	  __aio_remove_request (NULL, runp, 0);
	  if (runp->next_prio != NULL)
	    add_request_to_runlist (runp->next_prio);

	  /* Free the old element.  */
	  __aio_free_request (runp);
	}

      runp = runlist;

      /* If the runlist is empty, then we sleep for a while, waiting for
	 something to arrive in it. */
      if (runp == NULL && optim.aio_idle_time >= 0)
	{
	  struct timeval now;
	  struct timespec wakeup_time;

	  ++idle_thread_count;
	  gettimeofday (&now, NULL);
	  wakeup_time.tv_sec = now.tv_sec + optim.aio_idle_time;
	  wakeup_time.tv_nsec = now.tv_usec * 1000;
	  if (wakeup_time.tv_nsec > 1000000000)
	    {
	      wakeup_time.tv_nsec -= 1000000000;
	      ++wakeup_time.tv_sec;
	    }
	  pthread_cond_timedwait (&__aio_new_request_notification,
				  &__aio_requests_mutex,
				  &wakeup_time);
	  --idle_thread_count;
	  runp = runlist;
	}

      if (runp == NULL)
	--nthreads;
      else
	{
	  assert (runp->running == yes);
	  runp->running = allocated;
	  runlist = runp->next_run;

	  /* If we have a request to process, and there's still another in
	     the run list, then we need to either wake up or create a new
	     thread to service the request that is still in the run list. */
	  if (runlist != NULL)
	    {
	      /* There are at least two items in the work queue to work on.
		 If there are other idle threads, then we should wake them
		 up for these other work elements; otherwise, we should try
		 to create a new thread. */
	      if (idle_thread_count > 0)
		pthread_cond_signal (&__aio_new_request_notification);
	      else if (nthreads < optim.aio_threads)
		{
		  pthread_t thid;

		  /* Now try to start a thread. If we fail, no big deal,
		     because we know that there is at least one thread (us)
		     that is working on AIO operations. */
		  if (aio_create_helper_thread (&thid, handle_fildes_io, NULL)
		      == 0)
		    ++nthreads;
		}
	    }
	}

      /* Release the mutex.  */
      pthread_mutex_unlock (&__aio_requests_mutex);
    }
  while (runp != NULL);

  return NULL;
}


/* Free allocated resources.  */
libc_freeres_fn (free_res)
{
  size_t row;

  for (row = 0; row < pool_max_size; ++row)
    free (pool[row]);

  free (pool);
}


/* Add newrequest to the runlist. The __abs_prio flag of newrequest must
   be correctly set to do this. Also, you had better set newrequest's
   "running" flag to "yes" before you release your lock or you'll throw an
   assertion. */
static void
internal_function
add_request_to_runlist (struct requestlist *newrequest)
{
  int prio = newrequest->aiocbp->aiocb.__abs_prio;
  struct requestlist *runp;

  if (runlist == NULL || runlist->aiocbp->aiocb.__abs_prio < prio)
    {
      newrequest->next_run = runlist;
      runlist = newrequest;
    }
  else
    {
      runp = runlist;

      while (runp->next_run != NULL
	     && runp->next_run->aiocbp->aiocb.__abs_prio >= prio)
	runp = runp->next_run;

      newrequest->next_run = runp->next_run;
      runp->next_run = newrequest;
    }
}
#endif
