/* High-resolution sleep with the specified clock.
   Copyright (C) 2000, 2001 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <assert.h>
#include <errno.h>
#include <time.h>
#include <hp-timing.h>


#if HP_TIMING_AVAIL
# define CLOCK_P(clock) \
  (clock) != CLOCK_PROCESS_CPUTIME_ID					      \
  && (clock) != CLOCK_THREAD_CPUTIME_ID
#else
# define CLOCK_P(clock) 0
#endif


/* This implementation assumes that these is only a `nanosleep' system
   call.  So we have to remap all other activities.  */
int
clock_nanosleep (clockid_t clock_id, int flags, const struct timespec *req,
		 struct timespec *rem)
{
  struct timespec now;

  if (__builtin_expect (req->tv_nsec, 0) < 0
      || __builtin_expect (req->tv_nsec, 0) >= 1000000000)
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
    {
      /* Make sure the clock ID is correct.  */
      if (__builtin_expect (! CLOCK_P (clock_id), 0))
	return EINVAL;
    }

  return __builtin_expect (nanosleep (req, rem), 0) ? errno : 0;
}
