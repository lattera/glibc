/* Suspend until termination of a requests.
   Copyright (C) 1997,1998,1999,2000,2002,2003 Free Software Foundation, Inc.
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


/* We use an UGLY hack to prevent gcc from finding us cheating.  The
   implementations of aio_suspend and aio_suspend64 are identical and so
   we want to avoid code duplication by using aliases.  But gcc sees
   the different parameter lists and prints a warning.  We define here
   a function so that aio_suspend64 has no prototype.  */
#define aio_suspend64 XXX
#include <aio.h>
/* And undo the hack.  */
#undef aio_suspend64

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/time.h>

#include <bits/libc-lock.h>
#include "aio_misc.h"


struct clparam
{
  const struct aiocb *const *list;
  struct waitlist *waitlist;
  struct requestlist **requestlist;
  pthread_cond_t *cond;
  int nent;
};


static void
cleanup (void *arg)
{
  const struct clparam *param = (const struct clparam *) arg;

  /* Now remove the entry in the waiting list for all requests
     which didn't terminate.  */
  int cnt = param->nent;
  while (cnt-- > 0)
    if (param->list[cnt] != NULL
	&& param->list[cnt]->__error_code == EINPROGRESS)
      {
	struct waitlist **listp;

	assert (param->requestlist[cnt] != NULL);

	/* There is the chance that we cannot find our entry anymore. This
	   could happen if the request terminated and restarted again.  */
	listp = &param->requestlist[cnt]->waiting;
	while (*listp != NULL && *listp != &param->waitlist[cnt])
	  listp = &(*listp)->next;

	if (*listp != NULL)
	  *listp = (*listp)->next;
      }

  /* Release the conditional variable.  */
  (void) pthread_cond_destroy (param->cond);

  /* Release the mutex.  */
  pthread_mutex_unlock (&__aio_requests_mutex);
}


int
aio_suspend (list, nent, timeout)
     const struct aiocb *const list[];
     int nent;
     const struct timespec *timeout;
{
  struct waitlist waitlist[nent];
  struct requestlist *requestlist[nent];
  pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
  int cnt;
  bool any = false;
  int result = 0;
  int dummy;

  /* Request the mutex.  */
  pthread_mutex_lock (&__aio_requests_mutex);

  /* There is not yet a finished request.  Signal the request that
     we are working for it.  */
  for (cnt = 0; cnt < nent; ++cnt)
    if (list[cnt] != NULL)
      {
	if (list[cnt]->__error_code == EINPROGRESS)
	  {
	    requestlist[cnt] = __aio_find_req ((aiocb_union *) list[cnt]);

	    if (requestlist[cnt] != NULL)
	      {
		waitlist[cnt].cond = &cond;
		waitlist[cnt].next = requestlist[cnt]->waiting;
		waitlist[cnt].counterp = &dummy;
		waitlist[cnt].sigevp = NULL;
#ifdef BROKEN_THREAD_SIGNALS
		waitlist[cnt].caller_pid = 0;	/* Not needed.  */
#endif
		requestlist[cnt]->waiting = &waitlist[cnt];
		any = true;
	      }
	    else
	      /* We will never suspend.  */
	      break;
	  }
	else
	  /* We will never suspend.  */
	  break;
      }


  /* Only if none of the entries is NULL or finished to be wait.  */
  if (cnt == nent && any)
    {
      struct clparam clparam =
	{
	  .list = list,
	  .waitlist = waitlist,
	  .requestlist = requestlist,
	  .cond = &cond,
	  .nent = nent
	};

      pthread_cleanup_push (cleanup, &clparam);

      if (timeout == NULL)
	result = pthread_cond_wait (&cond, &__aio_requests_mutex);
      else
	{
	  /* We have to convert the relative timeout value into an
	     absolute time value with pthread_cond_timedwait expects.  */
	  struct timeval now;
	  struct timespec abstime;

	  __gettimeofday (&now, NULL);
	  abstime.tv_nsec = timeout->tv_nsec + now.tv_usec * 1000;
	  abstime.tv_sec = timeout->tv_sec + now.tv_sec;
	  if (abstime.tv_nsec >= 1000000000)
	    {
	      abstime.tv_nsec -= 1000000000;
	      abstime.tv_sec += 1;
	    }

	  result = pthread_cond_timedwait (&cond, &__aio_requests_mutex,
					   &abstime);
	}

      pthread_cleanup_pop (0);
    }

  /* Now remove the entry in the waiting list for all requests
     which didn't terminate.  */
  while (cnt-- > 0)
    if (list[cnt] != NULL && list[cnt]->__error_code == EINPROGRESS)
      {
	struct waitlist **listp;

	assert (requestlist[cnt] != NULL);

	/* There is the chance that we cannot find our entry anymore. This
	   could happen if the request terminated and restarted again.  */
	listp = &requestlist[cnt]->waiting;
	while (*listp != NULL && *listp != &waitlist[cnt])
	  listp = &(*listp)->next;

	if (*listp != NULL)
	  *listp = (*listp)->next;
      }

  /* Release the conditional variable.  */
  if (__builtin_expect (pthread_cond_destroy (&cond) != 0, 0))
    /* This must never happen.  */
    abort ();

  if (result != 0)
    {
      /* An error occurred.  Possibly it's EINTR.  We have to translate
	 the timeout error report of `pthread_cond_timedwait' to the
	 form expected from `aio_suspend'.  */
      if (result == ETIMEDOUT)
	__set_errno (EAGAIN);
      else
	__set_errno (result);

      result = -1;
    }

  /* Release the mutex.  */
  pthread_mutex_unlock (&__aio_requests_mutex);

  return result;
}

weak_alias (aio_suspend, aio_suspend64)
