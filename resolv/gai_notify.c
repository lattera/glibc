/* Copyright (C) 2001 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2001.

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

#include <netdb.h>
#include <pthread.h>
#include <stdlib.h>

#include "gai_misc.h"


static void *
notify_func_wrapper (void *arg)
{
  struct sigevent *sigev = arg;
  sigev->sigev_notify_function (sigev->sigev_value);
  return NULL;
}


int
internal_function
__gai_notify_only (struct sigevent *sigev, pid_t caller_pid)
{
  int result = 0;

  /* Send the signal to notify about finished processing of the request.  */
  if (sigev->sigev_notify == SIGEV_THREAD)
    {
      /* We have to start a thread.  */
      pthread_t tid;
      pthread_attr_t attr, *pattr;

      pattr = (pthread_attr_t *) sigev->sigev_notify_attributes;
      if (pattr == NULL)
	{
	  pthread_attr_init (&attr);
	  pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);
	  pattr = &attr;
	}

      if (pthread_create (&tid, pattr, notify_func_wrapper, sigev) < 0)
	result = -1;
    }
  else if (sigev->sigev_notify == SIGEV_SIGNAL)
    /* We have to send a signal.  */
    if (__gai_sigqueue (sigev->sigev_signo, sigev->sigev_value, caller_pid)
	< 0)
      result = -1;

  return result;
}


void
internal_function
__gai_notify (struct requestlist *req)
{
  struct waitlist *waitlist;

  /* Now also notify possibly waiting threads.  */
  waitlist = req->waiting;
  while (waitlist != NULL)
    {
      struct waitlist *next = waitlist->next;

      /* Decrement the counter.  This is used in both cases.  */
      --*waitlist->counterp;

      if (waitlist->sigevp == NULL)
	pthread_cond_signal (waitlist->cond);
      else
	/* This is part of a asynchronous `getaddrinfo_a' operation.  If
	   this request is the last one, send the signal.  */
	if (*waitlist->counterp == 0)
	  {
	    __gai_notify_only (waitlist->sigevp, waitlist->caller_pid);
	    /* This is tricky.  See getaddrinfo_a.c for the reason why
	       this works.  */
	    free ((void *) waitlist->counterp);
	  }

      waitlist = next;
    }
}
