/* Handle general operations.
   Copyright (C) 1997, 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1997.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <aio.h>
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

#include "aio_misc.h"

/* Pool of request list entries.  */
static struct requestlist **pool;

/* Number of total and allocated pool entries.  */
static size_t pool_tab_size;
static size_t pool_size;

/* We implement a two dimensional array but allocate each row separately.
   The macro below determines how many entries should be used per row.
   It should better be a power of two.  */
#define ENTRIES_PER_ROW	16

/* The row table is incremented in units of this.  */
#define ROW_STEP	8

/* List of available entries.  */
static struct requestlist *freelist;

/* List of request waiting to be processed.  */
static struct requestlist *runlist;

/* Structure list of all currently processed requests.  */
static struct requestlist *requests;

/* Number of threads currently running.  */
static int nthreads;


/* These are the values used to optimize the use of AIO.  The user can
   overwrite them by using the `aio_init' function.  */
static struct aioinit optim =
{
  20,	/* int aio_threads;	Maximal number of threads.  */
  256,	/* int aio_num;		Number of expected simultanious requests. */
  0,
  0,
  0,
  0,
  { 0, }
};


/* Since the list is global we need a mutex protecting it.  */
pthread_mutex_t __aio_requests_mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;


/* Functions to handle request list pool.  */
static struct requestlist *
get_elem (void)
{
  struct requestlist *result;

  if (freelist == NULL)
    {
      struct requestlist *new_row;
      size_t new_size;

      /* Compute new size.  */
      new_size = pool_size ? pool_size + ENTRIES_PER_ROW : optim.aio_num;

      if ((new_size / ENTRIES_PER_ROW) >= pool_tab_size)
	{
	  size_t new_tab_size = new_size / ENTRIES_PER_ROW;
	  struct requestlist **new_tab;

	  new_tab = (struct requestlist **)
	    realloc (pool, (new_tab_size * sizeof (struct requestlist *)));

	  if (new_tab == NULL)
	    return NULL;

	  pool_tab_size = new_tab_size;
	  pool = new_tab;
	}

      if (pool_size == 0)
	{
	  size_t cnt;

	  new_row = (struct requestlist *)
	    calloc (new_size, sizeof (struct requestlist));

	  if (new_row == NULL)
	    return NULL;

	  for (cnt = 0; cnt < new_size / ENTRIES_PER_ROW; ++cnt)
	    pool[cnt] = &new_row[cnt * ENTRIES_PER_ROW];
	}
      else
	{
	  /* Allocat one new row.  */
	  new_row = (struct requestlist *)
	    calloc (ENTRIES_PER_ROW, sizeof (struct requestlist));
	  if (new_row == NULL)
	    return NULL;

	  pool[new_size / ENTRIES_PER_ROW] = new_row;
	}

      /* Put all the new entries in the freelist.  */
      do
	{
	  new_row->next_prio = freelist;
	  freelist = new_row++;
	}
      while (++pool_size < new_size);
    }

  result = freelist;
  freelist = freelist->next_prio;

  return result;
}


void
__aio_free_request (struct requestlist *elem)
{
  elem->running = no;
  elem->next_prio = freelist;
  freelist = elem;
}


struct requestlist *
__aio_find_req (aiocb_union *elem)
{
  struct requestlist *runp = requests;
  int fildes = elem->aiocb.aio_fildes;

  while (runp != NULL && runp->aiocbp->aiocb.aio_fildes < fildes)
    runp = runp->next_fd;

  if (runp != NULL)
    if (runp->aiocbp->aiocb.aio_fildes != fildes)
      runp = NULL;
    else
      while (runp != NULL && runp->aiocbp != elem)
	runp = runp->next_prio;

  return runp;
}


struct requestlist *
__aio_find_req_fd (int fildes)
{
  struct requestlist *runp = requests;

  while (runp != NULL && runp->aiocbp->aiocb.aio_fildes < fildes)
    runp = runp->next_fd;

  return (runp != NULL && runp->aiocbp->aiocb.aio_fildes == fildes
	  ? runp : NULL);
}


/* The thread handler.  */
static void *handle_fildes_io (void *arg);


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

  /* Release the mutex.  */
  pthread_mutex_unlock (&__aio_requests_mutex);
}
weak_alias (__aio_init, aio_init)


/* The main function of the async I/O handling.  It enqueues requests
   and if necessary starts and handles threads.  */
struct requestlist *
__aio_enqueue_request (aiocb_union *aiocbp, int operation)
{
  int result = 0;
  int policy, prio;
  struct sched_param param;
  struct requestlist *last, *runp, *newp;
  int running = no;

  if (aiocbp->aiocb.aio_reqprio < 0
      || aiocbp->aiocb.aio_reqprio > AIO_PRIO_DELTA_MAX)
    {
      /* Invalid priority value.  */
      __set_errno (EINVAL);
      aiocbp->aiocb.__error_code = EINVAL;
      aiocbp->aiocb.__return_value = -1;
      return NULL;
    }

  /* Compute priority for this request.  */
  pthread_getschedparam (pthread_self (), &policy, &param);
  prio = param.sched_priority - aiocbp->aiocb.aio_reqprio;

  /* Get the mutex.  */
  pthread_mutex_lock (&__aio_requests_mutex);

  last = NULL;
  runp = requests;
  /* First look whether the current file descriptor is currently
     worked with.  */
  while (runp != NULL
	 && runp->aiocbp->aiocb.aio_fildes < aiocbp->aiocb.aio_fildes)
    {
      last = runp;
      runp = runp->next_fd;
    }

  /* Get a new element for the waiting list.  */
  newp = get_elem ();
  if (newp == NULL)
    {
      __set_errno (EAGAIN);
      pthread_mutex_unlock (&__aio_requests_mutex);
      return NULL;
    }
  newp->aiocbp = aiocbp;
  newp->waiting = NULL;

  aiocbp->aiocb.__abs_prio = prio;
  aiocbp->aiocb.__policy = policy;
  aiocbp->aiocb.aio_lio_opcode = operation;
  aiocbp->aiocb.__error_code = EINPROGRESS;
  aiocbp->aiocb.__return_value = 0;

  if (runp != NULL
      && runp->aiocbp->aiocb.aio_fildes == aiocbp->aiocb.aio_fildes)
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

      running = queued;
    }
  else
    {
      /* Enqueue this request for a new descriptor.  */
      if (last == NULL)
	{
	  newp->last_fd = NULL;
	  newp->next_fd = requests;
	  if (requests != NULL)
	    requests->last_fd = newp;
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
    }

  if (running == no)
    {
      /* We try to create a new thread for this file descriptor.  The
	 function which gets called will handle all available requests
	 for this descriptor and when all are processed it will
	 terminate.

	 If no new thread can be created or if the specified limit of
	 threads for AIO is reached we queue the request.  */

      /* See if we can create a thread.  */
      if (nthreads < optim.aio_threads)
	{
	  pthread_t thid;
	  pthread_attr_t attr;

	  /* Make sure the thread is created detached.  */
	  pthread_attr_init (&attr);
	  pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);

	  /* Now try to start a thread.  */
	  if (pthread_create (&thid, &attr, handle_fildes_io, newp) == 0)
	    {
	      /* We managed to enqueue the request.  All errors which can
		 happen now can be recognized by calls to `aio_return' and
		 `aio_error'.  */
	      running = allocated;
	      ++nthreads;
	    }
	  else if (nthreads == 0)
	    /* We cannot create a thread in the moment and there is
	       also no thread running.  This is a problem.  `errno' is
	       set to EAGAIN if this is only a temporary problem.  */
	    result = -1;
	}
    }

  /* Enqueue the request in the run queue if it is not yet running.  */
  if (running < yes && result == 0)
    {
      if (runlist == NULL || runlist->aiocbp->aiocb.__abs_prio < prio)
	{
	  newp->next_run = runlist;
	  runlist = newp;
	}
      else
	{
	  runp = runlist;

	  while (runp->next_run != NULL
		 && runp->next_run->aiocbp->aiocb.__abs_prio >= prio)
	    runp = runp->next_run;

	  newp->next_run = runp->next_run;
	  runp->next_run = newp;
	}
    }

  if (result == 0)
    newp->running = running;
  else
    {
      /* Something went wrong.  */
      __aio_free_request (newp);
      newp = NULL;
    }

  /* Release the mutex.  */
  pthread_mutex_unlock (&__aio_requests_mutex);

  return newp;
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
	      TEMP_FAILURE_RETRY (__pread64 (fildes,
					     (void *) aiocbp->aiocb64.aio_buf,
					     aiocbp->aiocb64.aio_nbytes,
					     aiocbp->aiocb64.aio_offset));
	  else
	    aiocbp->aiocb.__return_value =
	      TEMP_FAILURE_RETRY (pread (fildes,
					 (void *) aiocbp->aiocb.aio_buf,
					 aiocbp->aiocb.aio_nbytes,
					 aiocbp->aiocb.aio_offset));
	}
      else if ((aiocbp->aiocb.aio_lio_opcode & 127) == LIO_WRITE)
	{
	  if (aiocbp->aiocb.aio_lio_opcode & 128)
	    aiocbp->aiocb.__return_value =
	      TEMP_FAILURE_RETRY (__pwrite64 (fildes,
					      (const void *) aiocbp->aiocb64.aio_buf,
					      aiocbp->aiocb64.aio_nbytes,
					      aiocbp->aiocb64.aio_offset));
	  else
	    aiocbp->aiocb.__return_value =
	      TEMP_FAILURE_RETRY (pwrite (fildes,
					  (const void *) aiocbp->aiocb.aio_buf,
					  aiocbp->aiocb.aio_nbytes,
					  aiocbp->aiocb.aio_offset));
	}
      else if (aiocbp->aiocb.aio_lio_opcode == LIO_DSYNC)
	aiocbp->aiocb.__return_value = TEMP_FAILURE_RETRY (fdatasync (fildes));
      else if (aiocbp->aiocb.aio_lio_opcode == LIO_SYNC)
	aiocbp->aiocb.__return_value = TEMP_FAILURE_RETRY (fsync (fildes));
      else
	{
	  /* This is an invalid opcode.  */
	  aiocbp->aiocb.__return_value = -1;
	  __set_errno (EINVAL);
	}

      /* Get the mutex.  */
      pthread_mutex_lock (&__aio_requests_mutex);

      if (aiocbp->aiocb.__return_value == -1)
	aiocbp->aiocb.__error_code = errno;
      else
	aiocbp->aiocb.__error_code = 0;

      /* Send the signal to notify about finished processing of the
	 request.  */
      __aio_notify (runp);

      /* Now dequeue the current request.  */
      if (runp->next_prio == NULL)
	{
	  /* No outstanding request for this descriptor.  Remove this
	     descriptor from the list.  */
	  if (runp->next_fd != NULL)
	    runp->next_fd->last_fd = runp->last_fd;
	  if (runp->last_fd != NULL)
	    runp->last_fd->next_fd = runp->next_fd;
	}
      else
	{
	  runp->next_prio->last_fd = runp->last_fd;
	  runp->next_prio->next_fd = runp->next_fd;
	  runp->next_prio->running = yes;
	  if (runp->next_fd != NULL)
	    runp->next_fd->last_fd = runp->next_prio;
	  if (runp->last_fd != NULL)
	    runp->last_fd->next_fd = runp->next_prio;
	}

      /* Free the old element.  */
      __aio_free_request (runp);

      runp = runlist;
      if (runp != NULL)
	{
	  /* We must not run requests which are not marked `running'.  */
	  if (runp->running == yes)
	    runlist = runp->next_run;
	  else
	    {
	      struct requestlist *old;

	      do
		{
		  old = runp;
		  runp = runp->next_run;
		}
	      while (runp != NULL && runp->running != yes);

	      if (runp != NULL)
		old->next_run = runp->next_run;
	    }
	}

      /* If no request to work on we will stop the thread.  */
      if (runp == NULL)
	--nthreads;
      else
	runp->running = allocated;

      /* Release the mutex.  */
      pthread_mutex_unlock (&__aio_requests_mutex);
    }
  while (runp != NULL);

  pthread_exit (NULL);
}


/* Free allocated resources.  */
static void
__attribute__ ((unused))
free_res (void)
{
  size_t row;

  /* The first block of rows as specified in OPTIM is allocated in
     one chunk.  */
  free (pool[0]);

  for (row = optim.aio_num / ENTRIES_PER_ROW; row < pool_tab_size; ++row)
    free (pool[row]);

  free (pool);
}

text_set_element (__libc_subfreeres, free_res);
