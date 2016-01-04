/* futex operations for glibc-internal use.  Linux version.
   Copyright (C) 2014-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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

#ifndef FUTEX_INTERNAL_H
#define FUTEX_INTERNAL_H

#include <sysdeps/nptl/futex-internal.h>
#include <errno.h>
#include <lowlevellock-futex.h>
#include <nptl/pthreadP.h>

/* See sysdeps/nptl/futex-internal.h for documentation; this file only
   contains Linux-specific comments.

   The Linux kernel treats provides absolute timeouts based on the
   CLOCK_REALTIME clock and relative timeouts measured against the
   CLOCK_MONOTONIC clock.

   We expect a Linux kernel version of 2.6.22 or more recent (since this
   version, EINTR is not returned on spurious wake-ups anymore).  */

/* FUTEX_SHARED is always supported by the Linux kernel.  */
static __always_inline int
futex_supports_pshared (int pshared)
{
  if (__glibc_likely (pshared == PTHREAD_PROCESS_PRIVATE))
    return 0;
  else if (pshared == PTHREAD_PROCESS_SHARED)
    return 0;
  else
    return EINVAL;
}

/* The Linux kernel supports relative timeouts measured against the
   CLOCK_MONOTONIC clock.  */
static __always_inline bool
futex_supports_exact_relative_timeouts (void)
{
  return true;
}

/* See sysdeps/nptl/futex-internal.h for details.  */
static __always_inline int
futex_wait (unsigned int *futex_word, unsigned int expected, int private)
{
  int err = lll_futex_timed_wait (futex_word, expected, NULL, private);
  switch (err)
    {
    case 0:
    case -EAGAIN:
    case -EINTR:
      return -err;

    case -ETIMEDOUT: /* Cannot have happened as we provided no timeout.  */
    case -EFAULT: /* Must have been caused by a glibc or application bug.  */
    case -EINVAL: /* Either due to wrong alignment or due to the timeout not
		     being normalized.  Must have been caused by a glibc or
		     application bug.  */
    case -ENOSYS: /* Must have been caused by a glibc bug.  */
    /* No other errors are documented at this time.  */
    default:
      futex_fatal_error ();
    }
}

/* See sysdeps/nptl/futex-internal.h for details.  */
static __always_inline int
futex_wait_cancelable (unsigned int *futex_word, unsigned int expected,
		       int private)
{
  int oldtype;
  oldtype = __pthread_enable_asynccancel ();
  int err = lll_futex_timed_wait (futex_word, expected, NULL, private);
  __pthread_disable_asynccancel (oldtype);
  switch (err)
    {
    case 0:
    case -EAGAIN:
    case -EINTR:
      return -err;

    case -ETIMEDOUT: /* Cannot have happened as we provided no timeout.  */
    case -EFAULT: /* Must have been caused by a glibc or application bug.  */
    case -EINVAL: /* Either due to wrong alignment or due to the timeout not
		     being normalized.  Must have been caused by a glibc or
		     application bug.  */
    case -ENOSYS: /* Must have been caused by a glibc bug.  */
    /* No other errors are documented at this time.  */
    default:
      futex_fatal_error ();
    }
}

/* See sysdeps/nptl/futex-internal.h for details.  */
static __always_inline int
futex_reltimed_wait (unsigned int *futex_word, unsigned int expected,
		     const struct timespec *reltime, int private)
{
  int err = lll_futex_timed_wait (futex_word, expected, reltime, private);
  switch (err)
    {
    case 0:
    case -EAGAIN:
    case -EINTR:
    case -ETIMEDOUT:
      return -err;

    case -EFAULT: /* Must have been caused by a glibc or application bug.  */
    case -EINVAL: /* Either due to wrong alignment or due to the timeout not
		     being normalized.  Must have been caused by a glibc or
		     application bug.  */
    case -ENOSYS: /* Must have been caused by a glibc bug.  */
    /* No other errors are documented at this time.  */
    default:
      futex_fatal_error ();
    }
}

/* See sysdeps/nptl/futex-internal.h for details.  */
static __always_inline int
futex_reltimed_wait_cancelable (unsigned int *futex_word,
				unsigned int expected,
			        const struct timespec *reltime, int private)
{
  int oldtype;
  oldtype = __pthread_enable_asynccancel ();
  int err = lll_futex_timed_wait (futex_word, expected, reltime, private);
  __pthread_disable_asynccancel (oldtype);
  switch (err)
    {
    case 0:
    case -EAGAIN:
    case -EINTR:
    case -ETIMEDOUT:
      return -err;

    case -EFAULT: /* Must have been caused by a glibc or application bug.  */
    case -EINVAL: /* Either due to wrong alignment or due to the timeout not
		     being normalized.  Must have been caused by a glibc or
		     application bug.  */
    case -ENOSYS: /* Must have been caused by a glibc bug.  */
    /* No other errors are documented at this time.  */
    default:
      futex_fatal_error ();
    }
}

/* See sysdeps/nptl/futex-internal.h for details.  */
static __always_inline int
futex_abstimed_wait (unsigned int *futex_word, unsigned int expected,
		     const struct timespec *abstime, int private)
{
  /* Work around the fact that the kernel rejects negative timeout values
     despite them being valid.  */
  if (__glibc_unlikely ((abstime != NULL) && (abstime->tv_sec < 0)))
    return ETIMEDOUT;
  int err = lll_futex_timed_wait_bitset (futex_word, expected, abstime,
					 FUTEX_CLOCK_REALTIME, private);
  switch (err)
    {
    case 0:
    case -EAGAIN:
    case -EINTR:
    case -ETIMEDOUT:
      return -err;

    case -EFAULT: /* Must have been caused by a glibc or application bug.  */
    case -EINVAL: /* Either due to wrong alignment or due to the timeout not
		     being normalized.  Must have been caused by a glibc or
		     application bug.  */
    case -ENOSYS: /* Must have been caused by a glibc bug.  */
    /* No other errors are documented at this time.  */
    default:
      futex_fatal_error ();
    }
}

/* See sysdeps/nptl/futex-internal.h for details.  */
static __always_inline int
futex_abstimed_wait_cancelable (unsigned int *futex_word,
				unsigned int expected,
			        const struct timespec *abstime, int private)
{
  /* Work around the fact that the kernel rejects negative timeout values
     despite them being valid.  */
  if (__glibc_unlikely ((abstime != NULL) && (abstime->tv_sec < 0)))
    return ETIMEDOUT;
  int oldtype;
  oldtype = __pthread_enable_asynccancel ();
  int err = lll_futex_timed_wait_bitset (futex_word, expected, abstime,
					 FUTEX_CLOCK_REALTIME, private);
  __pthread_disable_asynccancel (oldtype);
  switch (err)
    {
    case 0:
    case -EAGAIN:
    case -EINTR:
    case -ETIMEDOUT:
      return -err;

    case -EFAULT: /* Must have been caused by a glibc or application bug.  */
    case -EINVAL: /* Either due to wrong alignment or due to the timeout not
		     being normalized.  Must have been caused by a glibc or
		     application bug.  */
    case -ENOSYS: /* Must have been caused by a glibc bug.  */
    /* No other errors are documented at this time.  */
    default:
      futex_fatal_error ();
    }
}

/* See sysdeps/nptl/futex-internal.h for details.  */
static __always_inline void
futex_wake (unsigned int *futex_word, int processes_to_wake, int private)
{
  int res = lll_futex_wake (futex_word, processes_to_wake, private);
  /* No error.  Ignore the number of woken processes.  */
  if (res >= 0)
    return;
  switch (res)
    {
    case -EFAULT: /* Could have happened due to memory reuse.  */
    case -EINVAL: /* Could be either due to incorrect alignment (a bug in
		     glibc or in the application) or due to memory being
		     reused for a PI futex.  We cannot distinguish between the
		     two causes, and one of them is correct use, so we do not
		     act in this case.  */
      return;
    case -ENOSYS: /* Must have been caused by a glibc bug.  */
    /* No other errors are documented at this time.  */
    default:
      futex_fatal_error ();
    }
}

#endif  /* futex-internal.h */
