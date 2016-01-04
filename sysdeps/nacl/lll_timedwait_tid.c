/* Timed waiting for thread death.  NaCl version.
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

int
__lll_timedwait_tid (int *tidp, const struct timespec *abstime)
{
  /* Reject invalid timeouts.  */
  if (__glibc_unlikely (abstime->tv_nsec < 0)
      || __glibc_unlikely (abstime->tv_nsec >= 1000000000))
    return EINVAL;

  /* Repeat until thread terminated.  */
  int tid;
  while ((tid = atomic_load_relaxed (tidp)) != 0)
    {
      /* See exit-thread.h for details.  */
      if (tid == NACL_EXITING_TID)
	/* The thread should now be in the process of exiting, so it will
	   finish quick enough that the timeout doesn't matter.  If any
	   thread ever stays in this state for long, there is something
	   catastrophically wrong.  */
	atomic_spin_nop ();
      else
	{
	  assert (tid > 0);

	  /* If *FUTEX == TID, wait until woken or timeout.  */
	  int err = __nacl_irt_futex.futex_wait_abs ((volatile int *) tidp,
						     tid, abstime);
	  if (err != 0)
	    {
	      if (__glibc_likely (err == ETIMEDOUT))
		return err;
	      assert (err == EAGAIN);
	    }
	}
    }

  return 0;
}
