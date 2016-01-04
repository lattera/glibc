/* Timed low level locking for pthread library.  NaCl version.
   Copyright (C) 2015-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <assert.h>
#include <atomic.h>
#include <errno.h>
#include <lowlevellock.h>
#include <sys/time.h>


/* This behaves the same as the generic version in nptl/.  It's simpler
   because it doesn't need to convert an absolute timeout to a relative
   one (and back again in the lll_futex_timed_wait macro).  */

int
__lll_timedlock_wait (int *futex, const struct timespec *abstime, int private)
{
  /* Reject invalid timeouts.  */
  if (__glibc_unlikely (abstime->tv_nsec < 0)
      || __glibc_unlikely (abstime->tv_nsec >= 1000000000))
    return EINVAL;

  /* Try locking.  */
  while (atomic_exchange_acq (futex, 2) != 0)
    {
      /* If *futex == 2, wait until woken or timeout.  */
      int err = __nacl_irt_futex.futex_wait_abs ((volatile int *) futex, 2,
						 abstime);
      if (err != 0)
	{
	  if (__glibc_likely (err == ETIMEDOUT))
	    return err;
	  assert (err == EAGAIN);
	}
    }

  return 0;
}
