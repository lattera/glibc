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

#include <stdlib.h>

#include "pthreadP.h"
#include <lowlevellock.h>


int
pthread_cond_timedwait (cond, mutex, abstime)
     pthread_cond_t *cond;
     pthread_mutex_t *mutex;
     const struct timespec *abstime;
{
  int result;
  int err;

  /* This function is a cancellation point.  Test before we potentially
     go to sleep.  */
  CANCELLATION_P (THREAD_SELF);

  /* Make sure the condition is modified atomically.  */
  lll_mutex_lock (cond->__data.__lock);

  /* Release the mutex.  This might fail.  */
  err = INTUSE(__pthread_mutex_unlock) (mutex);
  if (__builtin_expect (err != 0, 0))
    {
      lll_mutex_unlock (cond->__data.__lock);
      return err;
    }

  /* One more tread waiting.  */
  ++cond->__data.__nr_sleepers;

  /* The actual conditional variable implementation.  */
  result = lll_cond_timedwait (cond, abstime);

  if (--cond->__data.__nr_sleepers == 0)
    /* Forget about the current wakeups now that they are done.  */
    cond->__data.__nr_wakers = 0;

  /* Lose the condvar lock.  */
  lll_mutex_unlock (cond->__data.__lock);

  /* We have to get the mutex before returning.  */
  err = INTUSE(__pthread_mutex_lock) (mutex);
  if (err != 0)
    /* XXX Unconditionally overwrite the result of the wait?  */
    result = err;

  /* Cancellation handling.  */
  CANCELLATION_P (THREAD_SELF);

  return result;
}
