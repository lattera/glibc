/* low level locking for pthread library.  Generic futex-using version.
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
#include <sys/time.h>


void
__lll_lock_wait (int *futex, int val)
{
  do {
      lll_futex_wait (futex, val+1);
  } while ((val = __lll_add (futex, 1)) != 0);
  *futex = 2;
}
hidden_proto (__lll_lock_wait)


int
__lll_timedlock_wait (int *futex, int val, const struct timespec *abstime)
{
  int err;

  /* Reject invalid timeouts.  */
  if (abstime->tv_nsec >= 1000000000)
    return EINVAL;

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
	break;

      /* Wait.  */
      rt.tv_sec = sec;
      rt.tv_nsec = nsec;
      err = lll_futex_timed_wait (futex, val+1, &rt);
    } while (err == 0 && (val = __lll_add (futex, 1)) != 0);

  *futex = 2;
  return -err;
}
hidden_proto (__lll_timedlock_wait)


/* These don't get included in libc.so  */
#ifdef IS_IN_libpthread
int
lll_unlock_wake_cb (int *futex)
{
  __lll_add (futex, 1);
  return 0;
}
hidden_proto (lll_unlock_wake_cb)


int
__lll_timedwait_tid (int *tidp, const struct timespec *abstime)
{
  int tid;
  int err = 0;

  if (abstime == NULL || abstime->tv_nsec >= 1000000000)
    return EINVAL;

  /* Repeat until thread terminated.  */
  while ((tid = *tidp) != 0)
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
	break;

      /* Wait.  */
      rt.tv_sec = sec;
      rt.tv_nsec = nsec;

      /* Wait until thread terminates.  */
      err = lll_futex_timed_wait (tidp, tid, &rt);
      if (err != 0)
	break;
    }

  return -err;
}

hidden_proto (__lll_timedwait_tid)
#endif
