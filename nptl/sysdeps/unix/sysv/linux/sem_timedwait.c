/* sem_timedwait -- wait on a semaphore.  Generic futex-using version.
   Copyright (C) 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Paul Mackerras <paulus@au.ibm.com>, 2003.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <errno.h>
#include <sysdep.h>
#include <lowlevellock.h>
#include <internaltypes.h>
#include <semaphore.h>

#include <pthreadP.h>
#include <shlib-compat.h>


int
sem_timedwait (sem_t *sem, const struct timespec *abstime)
{
  /* First check for cancellation.  */
  CANCELLATION_P (THREAD_SELF);

  int *futex = (int *) sem;
  int val;
  int err;

  if (*futex > 0)
    {
      val = atomic_decrement_if_positive (futex);
      if (val > 0)
	return 0;
    }

  err = -EINVAL;
  if (abstime->tv_nsec < 0 || abstime->tv_nsec >= 1000000000)
    goto error_return;

  do
    {
      struct timeval tv;
      struct timespec rt;
      int sec, nsec;

      /* Get the current time.  */
      __gettimeofday (&tv, NULL);

      /* Compute relative timeout.  */
      sec = abstime->tv_sec - tv.tv_sec;
      nsec = abstime->tv_nsec - tv.tv_usec * 1000;
      if (nsec < 0)
	{
	  nsec += 1000000000;
	  --sec;
	}

      /* Already timed out?  */
      err = -ETIMEDOUT;
      if (sec < 0)
	goto error_return;

      /* Do wait.  */
      rt.tv_sec = sec;
      rt.tv_nsec = nsec;

      /* Enable asynchronous cancellation.  Required by the standard.  */
      int oldtype = __pthread_enable_asynccancel ();

      err = lll_futex_timed_wait (futex, 0, &rt);

      /* Disable asynchronous cancellation.  */
      __pthread_disable_asynccancel (oldtype);

      if (err != 0 && err != -EWOULDBLOCK)
	goto error_return;

      val = atomic_decrement_if_positive (futex);
    }
  while (val <= 0);

  return 0;

 error_return:
  __set_errno (-err);
  return -1;
}
