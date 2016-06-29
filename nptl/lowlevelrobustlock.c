/* Copyright (C) 2006-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Jakub Jelinek <jakub@redhat.com>, 2006.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <sysdep.h>
#include <lowlevellock.h>
#include <sys/time.h>
#include <pthreadP.h>
#include <kernel-features.h>


int
__lll_robust_lock_wait (int *futex, int private)
{
  int oldval = *futex;
  int tid = THREAD_GETMEM (THREAD_SELF, tid);

  /* If the futex changed meanwhile try locking again.  */
  if (oldval == 0)
    goto try;

  do
    {
      /* If the owner died, return the present value of the futex.  */
      if (__glibc_unlikely (oldval & FUTEX_OWNER_DIED))
	return oldval;

      /* Try to put the lock into state 'acquired, possibly with waiters'.  */
      int newval = oldval | FUTEX_WAITERS;
      if (oldval != newval
	  && atomic_compare_and_exchange_bool_acq (futex, newval, oldval))
	continue;

      /* If *futex == 2, wait until woken.  */
      lll_futex_wait (futex, newval, private);

    try:
      ;
    }
  while ((oldval = atomic_compare_and_exchange_val_acq (futex,
							tid | FUTEX_WAITERS,
							0)) != 0);
  return 0;
}


int
__lll_robust_timedlock_wait (int *futex, const struct timespec *abstime,
			     int private)
{
  /* Reject invalid timeouts.  */
  if (abstime->tv_nsec < 0 || abstime->tv_nsec >= 1000000000)
    return EINVAL;

  int tid = THREAD_GETMEM (THREAD_SELF, tid);
  int oldval = *futex;

  /* If the futex changed meanwhile, try locking again.  */
  if (oldval == 0)
    goto try;

  /* Work around the fact that the kernel rejects negative timeout values
     despite them being valid.  */
  if (__glibc_unlikely (abstime->tv_sec < 0))
    return ETIMEDOUT;

  do
    {
#if (!defined __ASSUME_FUTEX_CLOCK_REALTIME \
     || !defined lll_futex_timed_wait_bitset)
      struct timeval tv;
      struct timespec rt;

      /* Get the current time.  */
      (void) __gettimeofday (&tv, NULL);

      /* Compute relative timeout.  */
      rt.tv_sec = abstime->tv_sec - tv.tv_sec;
      rt.tv_nsec = abstime->tv_nsec - tv.tv_usec * 1000;
      if (rt.tv_nsec < 0)
	{
	  rt.tv_nsec += 1000000000;
	  --rt.tv_sec;
	}

      /* Already timed out?  */
      if (rt.tv_sec < 0)
	return ETIMEDOUT;
#endif

      /* If the owner died, return the present value of the futex.  */
      if (__glibc_unlikely (oldval & FUTEX_OWNER_DIED))
	return oldval;

      /* Try to put the lock into state 'acquired, possibly with waiters'.  */
      int newval = oldval | FUTEX_WAITERS;
      if (oldval != newval
	  && atomic_compare_and_exchange_bool_acq (futex, newval, oldval))
	continue;

      /* If *futex == 2, wait until woken or timeout.  */
#if (!defined __ASSUME_FUTEX_CLOCK_REALTIME \
     || !defined lll_futex_timed_wait_bitset)
      lll_futex_timed_wait (futex, newval, &rt, private);
#else
      int err = lll_futex_timed_wait_bitset (futex, newval, abstime,
					     FUTEX_CLOCK_REALTIME, private);
      /* The futex call timed out.  */
      if (err == -ETIMEDOUT)
         return -err;
#endif

    try:
      ;
    }
  while ((oldval = atomic_compare_and_exchange_val_acq (futex,
							tid | FUTEX_WAITERS,
							0)) != 0);

  return 0;
}
