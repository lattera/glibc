/* Copyright (C) 2002, 2003, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2002.

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

#include <errno.h>
#include <sched.h>
#include <string.h>
#include "pthreadP.h"
#include <lowlevellock.h>


int
__pthread_setschedparam (threadid, policy, param)
     pthread_t threadid;
     int policy;
     const struct sched_param *param;
{
  struct pthread *pd = (struct pthread *) threadid;

  /* Make sure the descriptor is valid.  */
  if (INVALID_TD_P (pd))
    /* Not a valid thread handle.  */
    return ESRCH;

  int result = 0;

  /* We have to handle cancellation in the following code since we are
     locking another threads desriptor.  */
  pthread_cleanup_push ((void (*) (void *)) lll_unlock_wake_cb, &pd->lock);

  lll_lock (pd->lock);

  /* Try to set the scheduler information.  */
  if (__builtin_expect (__sched_setscheduler (pd->tid, policy,
					      param) == -1, 0))
    result = errno;
  else
    {
      /* We succeeded changing the kernel information.  Reflect this
	 change in the thread descriptor.  */
      pd->schedpolicy = policy;
      memcpy (&pd->schedparam, param, sizeof (struct sched_param));
      pd->flags |= ATTR_FLAG_SCHED_SET | ATTR_FLAG_POLICY_SET;
    }

  lll_unlock (pd->lock);

  pthread_cleanup_pop (0);

  return result;
}
strong_alias (__pthread_setschedparam, pthread_setschedparam)
