/* Handle general operations.
   Copyright (C) 1997 Free Software Foundation, Inc.
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
#include <semaphore.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

#include "aio_misc.h"


/* We need a list of pending operations.  This is sorted according to
   the priority given in the aio_reqprio member.  */
aiocb_union *__aio_requests;

/* Since the list is global we need a semaphore protecting it.  */
sem_t __aio_requests_sema;


/* The initialization function.  It gets automatically called if any
   aio_* function is used in the program.  */
static void
__attribute__ ((unused))
aio_initialize (void)
{
  /* Initialize the semaphore.  We allow exactly one user at a time.  */
  sem_init (&__aio_requests_sema, 0, 1);
}

text_set_element (__libc_subinit, aio_initialize);


/* The thread handler.  */
static void *handle_fildes_io (void *arg);


/* The main function of the async I/O handling.  It enqueues requests
   and if necessary starts and handles threads.  */
int
__aio_enqueue_request (aiocb_union *aiocbp, int operation, int require_lock)
{
  int result;
  int policy, prio;
  struct sched_param param;
  aiocb_union *runp;

  if (aiocbp->aiocb.aio_reqprio < 0
      || aiocbp->aiocb.aio_reqprio > AIO_PRIO_DELTA_MAX)
    {
      /* Invalid priority value.  */
      __set_errno (EINVAL);
      aiocbp->aiocb.__error_code = EINVAL;
      aiocbp->aiocb.__return_value = -1;
      return -1;
    }

  if (pthread_getschedparam (pthread_self (), &policy, &param) < 0)
    {
      /* Something went wrong.  */
      aiocbp->aiocb.__error_code = errno;
      aiocbp->aiocb.__return_value = -1;
      return -1;
    }

  /* Compute priority for this request.  */
  prio = param.sched_priority - aiocbp->aiocb.aio_reqprio;


  /* Get the semaphore.  */
  if (require_lock)
    sem_wait (&__aio_requests_sema);

  runp = __aio_requests;
  /* First look whether the current file descriptor is currently
     worked with.  */
  while (runp != NULL && runp->aiocb.aio_fildes < aiocbp->aiocb.aio_fildes)
    runp = (aiocb_union *) runp->aiocb.__next_fd;

  if (runp != NULL)
    {
      /* The current file descriptor is worked on.  It makes no sense
	 to start another thread since this new thread would have to
	 wait for the previous one to terminate.  Simply enqueue it
	 after the running one according to the priority.  */
      while (runp->aiocb.__next_prio != NULL
	     && runp->aiocb.__next_prio->__abs_prio >= prio)
	runp = (aiocb_union *) runp->aiocb.__next_prio;

      aiocbp->aiocb.__next_prio = runp->aiocb.__next_prio;
      aiocbp->aiocb.__abs_prio = prio;
      aiocbp->aiocb.__policy = policy;
      aiocbp->aiocb.aio_lio_opcode = operation;
      aiocbp->aiocb.__error_code = EINPROGRESS;
      aiocbp->aiocb.__return_value = 0;
      runp->aiocb.__next_prio = (struct aiocb *) aiocbp;

      result = 0;
    }
  else
    {
      /* We create a new thread for this file descriptor.  The
	 function which gets called will handle all available requests
	 for this descriptor and when all are processed it will
	 terminate.  */
      pthread_t thid;
      pthread_attr_t attr;

      /* First enqueue the request (the list is empty).  */
      aiocbp->aiocb.__next_fd = NULL;
      aiocbp->aiocb.__last_fd = NULL;

      aiocbp->aiocb.__next_prio = NULL;
      aiocbp->aiocb.__abs_prio = prio;
      aiocbp->aiocb.__policy = policy;
      aiocbp->aiocb.aio_lio_opcode = operation;
      aiocbp->aiocb.__error_code = EINPROGRESS;
      aiocbp->aiocb.__return_value = 0;

      /* Make sure the thread is created detached.  */
      pthread_attr_init (&attr);
      pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);

      /* Now try to start a thread.  */
      if (pthread_create (&thid, &attr, handle_fildes_io, aiocbp) < 0)
	{
	  result = -1;
	  aiocbp->aiocb.__error_code = errno;
	  aiocbp->aiocb.__return_value = -1;
	}
      else
	/* We managed to enqueue the request.  All errors which can
	   happen now can be recognized by calls to `aio_return' and
	   `aio_error'.  */
	  result = 0;
    }

  /* Release the semaphore.  */
  if (require_lock)
    sem_post (&__aio_requests_sema);

  return result;
}


static void *
handle_fildes_io (void *arg)
{
  pthread_t self = pthread_self ();
  struct sched_param param;
  aiocb_union *runp = (aiocb_union *) arg;
  int policy;
  int fildes = runp->aiocb.aio_fildes;	/* This is always the same.  */

  pthread_getschedparam (self, &policy, &param);

  do
    {
      /* Change the priority to the requested value (if necessary).  */
      if (runp->aiocb.__abs_prio != param.sched_priority
	  || runp->aiocb.__policy != policy)
	{
	  param.sched_priority = runp->aiocb.__abs_prio;
	  policy = runp->aiocb.__policy;
	  pthread_setschedparam (self, policy, &param);
	}

      /* Process request pointed to by RUNP.  We must not be disturbed
	 by signals.  */
      if ((runp->aiocb.aio_lio_opcode & 127) == LIO_READ)
	{
	  if (runp->aiocb.aio_lio_opcode & 128)
	    runp->aiocb.__return_value =
	      TEMP_FAILURE_RETRY (__pread64 (fildes,
					     (void *) runp->aiocb64.aio_buf,
					     runp->aiocb64.aio_nbytes,
					     runp->aiocb64.aio_offset));
	  else
	    runp->aiocb.__return_value =
	      TEMP_FAILURE_RETRY (__pread (fildes,
					   (void *) runp->aiocb.aio_buf,
					   runp->aiocb.aio_nbytes,
					   runp->aiocb.aio_offset));
	}
      else if ((runp->aiocb.aio_lio_opcode & 127) == LIO_WRITE)
	{
	  if (runp->aiocb.aio_lio_opcode & 128)
	    runp->aiocb.__return_value =
	      TEMP_FAILURE_RETRY (__pwrite64 (fildes,
					      (const void *) runp->aiocb64.aio_buf,
					      runp->aiocb64.aio_nbytes,
					      runp->aiocb64.aio_offset));
	  else
	    runp->aiocb.__return_value =
	      TEMP_FAILURE_RETRY (__pwrite (fildes,
					    (const void *) runp->aiocb.aio_buf,
					    runp->aiocb.aio_nbytes,
					    runp->aiocb.aio_offset));
	}
      else if (runp->aiocb.aio_lio_opcode == __LIO_DSYNC)
	runp->aiocb.__return_value = TEMP_FAILURE_RETRY (fdatasync (fildes));
      else if (runp->aiocb.aio_lio_opcode == __LIO_SYNC)
	runp->aiocb.__return_value = TEMP_FAILURE_RETRY (fsync (fildes));
      else
	{
	  /* This is an invalid opcode.  */
	  runp->aiocb.__return_value = -1;
	  __set_errno (EINVAL);
	}

      if (runp->aiocb.__return_value == -1)
	runp->aiocb.__error_code = errno;
      else
	runp->aiocb.__error_code = 0;

      /* Send the signal to notify about finished processing of the
	 request.  */
      if (runp->aiocb.aio_sigevent.sigev_notify == SIGEV_THREAD)
	{
	  /* We have to start a thread.  */
	  pthread_t tid;
	  pthread_attr_t attr, *pattr;

	  pattr = (pthread_attr_t *)
	    runp->aiocb.aio_sigevent.sigev_notify_attributes;
	  if (pattr == NULL)
	    {
	      pthread_attr_init (&attr);
	      pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);
	      pattr = &attr;
	    }

	  if (pthread_create (&tid,
			      (pthread_attr_t *)
			      runp->aiocb.aio_sigevent.sigev_notify_attributes,
			      (void *(*) (void *))
			      runp->aiocb.aio_sigevent.sigev_notify_function,
			      runp->aiocb.aio_sigevent.sigev_value.sival_ptr)
	      < 0)
	    {
	      /* XXX What shall we do if already an error is set by
		 read/write/fsync?  */
	      runp->aiocb.__error_code = errno;
	      runp->aiocb.__return_value = -1;
	    }
	}
      else if (runp->aiocb.aio_sigevent.sigev_notify == SIGEV_SIGNAL)
	/* We have to send a signal.  */
	if (__aio_sigqueue (runp->aiocb.aio_sigevent.sigev_signo,
			    runp->aiocb.aio_sigevent.sigev_value) < 0)
	  {
	    /* XXX What shall we do if already an error is set by
	       read/write/fsync?  */
	    runp->aiocb.__error_code = errno;
	    runp->aiocb.__return_value = -1;
	  }

      /* Get the semaphore.  */
      sem_wait (&__aio_requests_sema);

      /* Now dequeue the current request.  */
      if (runp->aiocb.__next_prio == NULL)
	{
	  if (runp->aiocb.__next_fd != NULL)
	    runp->aiocb.__next_fd->__last_fd = runp->aiocb.__last_fd;
	  if (runp->aiocb.__last_fd != NULL)
	    runp->aiocb.__last_fd->__next_fd = runp->aiocb.__next_fd;
	  runp = NULL;
	}
      else
	{
	  runp->aiocb.__next_prio->__last_fd = runp->aiocb.__last_fd;
	  runp->aiocb.__next_prio->__next_fd = runp->aiocb.__next_fd;
	  if (runp->aiocb.__next_fd != NULL)
	    runp->aiocb.__next_fd->__last_fd = runp->aiocb.__next_prio;
	  if (runp->aiocb.__last_fd != NULL)
	    runp->aiocb.__last_fd->__next_fd = runp->aiocb.__next_prio;
	  runp = (aiocb_union *) runp->aiocb.__next_prio;
	}

      /* Release the semaphore.  */
      sem_post (&__aio_requests_sema);
    }
  while (runp != NULL);

  pthread_exit (NULL);
}
