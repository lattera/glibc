/* Cancel requests associated with given file descriptor.
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


/* We use an UGLY hack to prevent gcc from finding us cheating.  The
   implementation of aio_cancel and aio_cancel64 are identical and so
   we want to avoid code duplication by using aliases.  But gcc sees
   the different parameter lists and prints a warning.  We define here
   a function so that aio_cancel64 has no prototype.  */
#define aio_cancel64 XXX
#include <aio.h>
/* And undo the hack.  */
#undef aio_cancel64

#include <errno.h>
#include <pthread.h>

#include "aio_misc.h"


/* Argh, so far there is no ECANCELED.  */
#define ECANCELED 125

int
aio_cancel (fildes, aiocbp)
     int fildes;
     struct aiocb *aiocbp;
{
  struct aiocb *firstp;
  int result = AIO_ALLDONE;

  /* Request the semaphore.  */
  sem_wait (&__aio_requests_sema);

  /* Search for the list of requests associated with the given file
     descriptor.  */
  for (firstp = (struct aiocb *) __aio_requests; firstp != NULL;
       firstp = firstp->__next_fd)
    if (firstp->aio_fildes == fildes)
      break;

  /* If the file descriptor is not found all work seems to done
     already.  Otherwise try to cancel the request(s).  */
  if (firstp != NULL)
    {
      if (aiocbp != NULL)
	{
	  /* Locate the entry corresponding to the AIOCBP parameter.  */
	  if (aiocbp == firstp)
	    /* The requests is currently handled, therefore don't
	       cancel it and signal this to the user.  */
	    result = AIO_NOTCANCELED;
	  else
	    {
	      while (firstp->__next_prio != NULL
		     && aiocbp != firstp->__next_prio)
		firstp = firstp->__next_prio;

	      if (firstp->__next_prio != NULL)
		{
		  /* The request the user wants to cancel is in the
		     queue.  Simply remove it.  */
		  firstp->__next_prio = aiocbp->__next_prio;

		  /* Mark as canceled.  */
		  aiocbp->__error_code = ECANCELED;
		  aiocbp->__return_value = -1;

		  /* Send the signal to notify about canceled
		     processing of the request.  */
		  if (aiocbp->aio_sigevent.sigev_notify == SIGEV_THREAD)
		    {
		      /* We have to start a thread.  */
		      pthread_t tid;
		      pthread_attr_t attr, *pattr;

		      pattr = (pthread_attr_t *)
			aiocbp->aio_sigevent.sigev_notify_attributes;
		      if (pattr == NULL)
			{
			  pthread_attr_init (&attr);
			  pthread_attr_setdetachstate (&attr,
						       PTHREAD_CREATE_DETACHED);
			  pattr = &attr;
			}

		      pthread_create (&tid, pattr,
				      (void *(*) (void *))
				      aiocbp->aio_sigevent.sigev_notify_function,
				      aiocbp->aio_sigevent.sigev_value.sival_ptr);
		    }
		  else if (aiocbp->aio_sigevent.sigev_notify == SIGEV_SIGNAL)
		    /* We have to send a signal.  */
		    __aio_sigqueue (aiocbp->aio_sigevent.sigev_signo,
				    aiocbp->aio_sigevent.sigev_value);

		  result = AIO_CANCELED;
		}
	    }
	}
      else
	{
	  /* First dequeue all waiting requests.  */
	  aiocbp = firstp;

	  while ((firstp = firstp->__next_prio) != NULL)
	    {
	      firstp->__error_code = ECANCELED;
	      firstp->__return_value = -1;


	      /* Send the signal to notify about canceled processing
		 of the request.  */
	      if (firstp->aio_sigevent.sigev_notify == SIGEV_THREAD)
		{
		  /* We have to start a thread.  */
		  pthread_t tid;
		  pthread_attr_t attr, *pattr;

		  pattr = (pthread_attr_t *)
		    aiocbp->aio_sigevent.sigev_notify_attributes;
		  if (pattr == NULL)
		    {
		      pthread_attr_init (&attr);
		      pthread_attr_setdetachstate (&attr,
						   PTHREAD_CREATE_DETACHED);
		      pattr = &attr;
		    }

		  pthread_create (&tid, pattr,
				  (void *(*) (void *))
				  firstp->aio_sigevent.sigev_notify_function,
				  firstp->aio_sigevent.sigev_value.sival_ptr);
		}
	      else if (firstp->aio_sigevent.sigev_notify == SIGEV_SIGNAL)
		/* We have to send a signal.  */
		__aio_sigqueue (firstp->aio_sigevent.sigev_signo,
				firstp->aio_sigevent.sigev_value);
	    }

	  /* We have to signal that not all requests could be canceled
	     since the first requests is currently processed.  */
	  result = AIO_NOTCANCELED;

	  aiocbp->__next_prio = NULL;
	}
    }

  /* Release the semaphore.  */
  sem_post (&__aio_requests_sema);

  return result;
}

weak_alias (aio_cancel, aio_cancel64)
