/* Enqueue and list of read or write requests.
   Copyright (C) 1997,1998,1999,2000,2001,2003 Free Software Foundation, Inc.
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

#ifndef lio_listio
#include <aio.h>
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#include "aio_misc.h"

#define LIO_OPCODE_BASE 0
#endif

/* We need this special structure to handle asynchronous I/O.  */
struct async_waitlist
  {
    int counter;
    struct sigevent sigev;
    struct waitlist list[0];
  };


int
lio_listio (mode, list, nent, sig)
     int mode;
     struct aiocb *const list[];
     int nent;
     struct sigevent *sig;
{
  struct sigevent defsigev;
  struct requestlist *requests[nent];
  int cnt;
  volatile int total = 0;
  int result = 0;

  /* Check arguments.  */
  if (mode != LIO_WAIT && mode != LIO_NOWAIT)
    {
      __set_errno (EINVAL);
      return -1;
    }

  if (sig == NULL)
    {
      defsigev.sigev_notify = SIGEV_NONE;
      sig = &defsigev;
    }

  /* Request the mutex.  */
  pthread_mutex_lock (&__aio_requests_mutex);

  /* Now we can enqueue all requests.  Since we already acquired the
     mutex the enqueue function need not do this.  */
  for (cnt = 0; cnt < nent; ++cnt)
    if (list[cnt] != NULL && list[cnt]->aio_lio_opcode != LIO_NOP)
      {
	list[cnt]->aio_sigevent.sigev_notify = SIGEV_NONE;
	requests[cnt] = __aio_enqueue_request ((aiocb_union *) list[cnt],
					       (list[cnt]->aio_lio_opcode
					        | LIO_OPCODE_BASE));

	if (requests[cnt] != NULL)
	  /* Successfully enqueued.  */
	  ++total;
	else
	  /* Signal that we've seen an error.  `errno' and the error code
	     of the aiocb will tell more.  */
	  result = -1;
      }
    else
      requests[cnt] = NULL;

  if (total == 0)
    {
      /* We don't have anything to do except signalling if we work
	 asynchronously.  */

      /* Release the mutex.  We do this before raising a signal since the
	 signal handler might do a `siglongjmp' and then the mutex is
	 locked forever.  */
      pthread_mutex_unlock (&__aio_requests_mutex);

      if (mode == LIO_NOWAIT)
	{
#ifdef BROKEN_THREAD_SIGNALS
	__aio_notify_only (sig,
			   sig->sigev_notify == SIGEV_SIGNAL ? getpid () : 0);
#else
	__aio_notify_only (sig);
#endif
	}

      return result;
    }
  else if (mode == LIO_WAIT)
    {
      pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
      struct waitlist waitlist[nent];
      int oldstate;

      total = 0;
      for (cnt = 0; cnt < nent; ++cnt)
	{
	  assert (requests[cnt] == NULL || list[cnt] != NULL);

	  if (requests[cnt] != NULL && list[cnt]->aio_lio_opcode != LIO_NOP)
	    {
	      waitlist[cnt].cond = &cond;
	      waitlist[cnt].next = requests[cnt]->waiting;
	      waitlist[cnt].counterp = &total;
	      waitlist[cnt].sigevp = NULL;
#ifdef BROKEN_THREAD_SIGNALS
	      waitlist[cnt].caller_pid = 0;	/* Not needed.  */
#endif
	      requests[cnt]->waiting = &waitlist[cnt];
	      ++total;
	    }
	}

      /* Since `pthread_cond_wait'/`pthread_cond_timedwait' are cancelation
	 points we must be careful.  We added entries to the waiting lists
	 which we must remove.  So defer cancelation for now.  */
      pthread_setcancelstate (PTHREAD_CANCEL_DISABLE, &oldstate);

      while (total > 0)
	pthread_cond_wait (&cond, &__aio_requests_mutex);

      /* Now it's time to restore the cancelation state.  */
      pthread_setcancelstate (oldstate, NULL);

      /* Release the conditional variable.  */
      if (pthread_cond_destroy (&cond) != 0)
	/* This must never happen.  */
	abort ();
    }
  else
    {
      struct async_waitlist *waitlist;

      waitlist = (struct async_waitlist *)
	malloc (sizeof (struct async_waitlist)
		+ (nent * sizeof (struct waitlist)));

      if (waitlist == NULL)
	{
	  __set_errno (EAGAIN);
	  result = -1;
	}
      else
	{
#ifdef BROKEN_THREAD_SIGNALS
	  pid_t caller_pid = sig->sigev_notify == SIGEV_SIGNAL ? getpid () : 0;
#endif
	  total = 0;

	  for (cnt = 0; cnt < nent; ++cnt)
	    {
	      assert (requests[cnt] == NULL || list[cnt] != NULL);

	      if (requests[cnt] != NULL
		  && list[cnt]->aio_lio_opcode != LIO_NOP)
		{
		  waitlist->list[cnt].cond = NULL;
		  waitlist->list[cnt].next = requests[cnt]->waiting;
		  waitlist->list[cnt].counterp = &waitlist->counter;
		  waitlist->list[cnt].sigevp = &waitlist->sigev;
#ifdef BROKEN_THREAD_SIGNALS
		  waitlist->list[cnt].caller_pid = caller_pid;
#endif
		  requests[cnt]->waiting = &waitlist->list[cnt];
		  ++total;
		}
	    }

	  waitlist->counter = total;
	  waitlist->sigev = *sig;
	}
    }

  /* Release the mutex.  */
  pthread_mutex_unlock (&__aio_requests_mutex);

  return result;
}
