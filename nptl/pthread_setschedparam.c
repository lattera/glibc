/* Copyright (C) 2002 Free Software Foundation, Inc.
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
pthread_setschedparam (thread_id, policy, param)
     pthread_t thread_id;
     int policy;
     const struct sched_param *param;
{
  struct pthread *thread = (struct pthread *) thread_id;
  int result = 0;

  /* We have to handle cancellation in the following code since we are
     locking another threads desriptor.  */
  pthread_cleanup_push ((void (*) (void *)) lll_unlock_wake_cb, &thread->lock);

  lll_lock (thread->lock);

  /* Try to set the scheduler information.  */
  if (__builtin_expect (__sched_setscheduler (thread->tid, policy,
					      param) == -1, 0))
    result = errno;
  else
    {
      /* We succeeded changing the kernel information.  Reflect this
	 change in the thread descriptor.  */
      thread->schedpolicy = policy;
      memcpy (&thread->schedparam, param, sizeof (struct sched_param));
    }

  lll_unlock (thread->lock);

  pthread_cleanup_pop (0);

  return result;
}
