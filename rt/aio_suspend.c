/* Suspend until termination of a requests.
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


/* We use an UGLY hack to prevent gcc from finding us cheating.  The
   implementation of aio_suspend and aio_suspend64 are identical and so
   we want to avoid code duplication by using aliases.  But gcc sees
   the different parameter lists and prints a warning.  We define here
   a function so that aio_suspend64 has no prototype.  */
#define aio_suspend64 XXX
#include <aio.h>
/* And undo the hack.  */
#undef aio_suspend64

#include <errno.h>

#include "aio_misc.h"


int
aio_suspend (list, nent, timeout)
     const struct aiocb *const list[];
     int nent;
     const struct timespec *timeout;
{
  struct waitlist waitlist[nent];
  struct requestlist *requestlist[nent];
  pthread_cond_t cond;
  int cnt;
  int result = 0;
  int dummy;
  int none = 1;

  /* Request the mutex.  */
  pthread_mutex_lock (&__aio_requests_mutex);

  /* There is not yet a finished request.  Signal the request that
     we are working for it.  */
  for (cnt = 0; cnt < nent; ++cnt)
    if (list[cnt] != NULL && list[cnt]->__error_code == EINPROGRESS)
      {
	requestlist[cnt] = __aio_find_req ((aiocb_union *) list[cnt]);

	if (requestlist[cnt] != NULL)
	  {
	    waitlist[cnt].cond = &cond;
	    waitlist[cnt].next = requestlist[cnt]->waiting;
	    waitlist[cnt].counterp = &dummy;
	    waitlist[cnt].sigevp = NULL;
	    requestlist[cnt]->waiting = &waitlist[cnt];
	    none = 0;
	  }
      }

  /* If there is a not finished request wait for it.  */
  if (!none)
    {
      int oldstate;

      /* Initialize the conditional variable.  */
      pthread_cond_init (&cond, NULL);

      /* Since `pthread_cond_wait'/`pthread_cond_timedwait' are cancelation
	 points we must be careful.  We added entries to the waiting lists
	 which we must remove.  So defer cancelation for now.  */
      pthread_setcancelstate (PTHREAD_CANCEL_DISABLE, &oldstate);

      if (timeout == NULL)
	result = pthread_cond_wait (&cond, &__aio_requests_mutex);
      else
	result = pthread_cond_timedwait (&cond, &__aio_requests_mutex,
					 timeout);

      /* Now remove the entry in the waiting list for all requests
	 which didn't terminate.  */
      for (cnt = 0; cnt < nent; ++cnt)
	if (list[cnt] != NULL && list[cnt]->__error_code == EINPROGRESS
	    && requestlist[cnt] != NULL)
	  {
	    struct waitlist **listp = &requestlist[cnt]->waiting;

	    /* There is the chance that we cannot find our entry anymore.
	       This could happen if the request terminated and restarted
	       again.  */
	    while (*listp != NULL && *listp != &waitlist[cnt])
	      listp = &(*listp)->next;

	    if (*listp != NULL)
	      *listp = (*listp)->next;
	  }

      /* Now it's time to restore the cancelation state.  */
      pthread_setcancelstate (oldstate, NULL);

      /* Release the conditional variable.  */
      if (pthread_cond_destroy (&cond) != 0)
	/* This must never happen.  */
	abort ();

      if (result != 0)
	{
	  /* An error occurred.  Possibly it's EINTR.  We have to translate
	     the timeout error report of `pthread_cond_timedwait' to the
	     form expected from `aio_suspend'.  */
	  if (result == ETIMEDOUT)
	    __set_errno (EAGAIN);

	  result = -1;
	}
    }

  /* Release the mutex.  */
  pthread_mutex_unlock (&__aio_requests_mutex);

  return result;
}

weak_alias (aio_suspend, aio_suspend64)
