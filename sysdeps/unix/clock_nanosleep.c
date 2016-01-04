/* High-resolution sleep with the specified clock.
   Copyright (C) 2000-2016 Free Software Foundation, Inc.
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
#include <errno.h>
#include <time.h>
#include <hp-timing.h>
#include <sysdep-cancel.h>

#if HP_TIMING_AVAIL
# define CPUCLOCK_P(clock) \
  ((clock) == CLOCK_PROCESS_CPUTIME_ID					      \
   || ((clock) & ((1 << CLOCK_IDFIELD_SIZE) - 1)) == CLOCK_THREAD_CPUTIME_ID)
#else
# define CPUCLOCK_P(clock) 0
#endif

#ifndef INVALID_CLOCK_P
# define INVALID_CLOCK_P(cl) \
  ((cl) < CLOCK_REALTIME || (cl) > CLOCK_THREAD_CPUTIME_ID)
#endif


/* This implementation assumes that these is only a `nanosleep' system
   call.  So we have to remap all other activities.  */
int
__clock_nanosleep (clockid_t clock_id, int flags, const struct timespec *req,
		   struct timespec *rem)
{
  struct timespec now;

  if (__builtin_expect (req->tv_nsec, 0) < 0
      || __builtin_expect (req->tv_nsec, 0) >= 1000000000)
    return EINVAL;

  if (clock_id == CLOCK_THREAD_CPUTIME_ID)
    return EINVAL;		/* POSIX specifies EINVAL for this case.  */

#ifdef SYSDEP_NANOSLEEP
  SYSDEP_NANOSLEEP;
#endif

  if (CPUCLOCK_P (clock_id))
    return ENOTSUP;

  if (INVALID_CLOCK_P (clock_id))
    return EINVAL;

  /* If we got an absolute time, remap it.  */
  if (flags == TIMER_ABSTIME)
    {
      long int nsec;
      long int sec;

      /* Make sure we use safe data types.  */
      assert (sizeof (sec) >= sizeof (now.tv_sec));

      /* Get the current time for this clock.  */
      if (__builtin_expect (clock_gettime (clock_id, &now), 0) != 0)
	return errno;

      /* Compute the difference.  */
      nsec = req->tv_nsec - now.tv_nsec;
      sec = req->tv_sec - now.tv_sec - (nsec < 0);
      if (sec < 0)
	/* The time has already elapsed.  */
	return 0;

      now.tv_sec = sec;
      now.tv_nsec = nsec + (nsec < 0 ? 1000000000 : 0);

      /* From now on this is our time.  */
      req = &now;

      /* Make sure we are not modifying the struct pointed to by REM.  */
      rem = NULL;
    }
  else if (__builtin_expect (flags, 0) != 0)
    return EINVAL;
  else if (clock_id != CLOCK_REALTIME)
    /* Not supported.  */
    return ENOTSUP;

  return __builtin_expect (nanosleep (req, rem), 0) ? errno : 0;
}
weak_alias (__clock_nanosleep, clock_nanosleep)
