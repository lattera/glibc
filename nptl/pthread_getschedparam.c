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

#include <string.h>
#include "pthreadP.h"
#include <lowlevellock.h>


int
__pthread_getschedparam (thread_id, policy, param)
     pthread_t thread_id;
     int *policy;
     struct sched_param *param;
{
  struct pthread *thread = (struct pthread *) thread_id;

  /* We have to handle cancellation in the following code since we are
     locking another threads desriptor.  */
  pthread_cleanup_push ((void (*) (void *)) lll_unlock_wake_cb, &thread->lock);

  lll_lock (thread->lock);

  /* The library is responsible for maintaining the values at all
     times.  If the user uses a interface other than
     pthread_setschedparam to modify the scheduler setting it is not
     the library's problem.  */
  *policy = thread->schedpolicy;
  memcpy (param, &thread->schedparam, sizeof (struct sched_param));

  lll_unlock (thread->lock);

  pthread_cleanup_pop (0);

  return 0;
}
strong_alias (__pthread_getschedparam, pthread_getschedparam)
