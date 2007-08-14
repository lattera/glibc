/* Copyright (C) 1999-2004, 2006, 2007 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <libc-internal.h>
#include <ldsodefs.h>


#if HP_TIMING_AVAIL && !defined HANDLED_CPUTIME
/* Clock frequency of the processor.  We make it a 64-bit variable
   because some jokers are already playing with processors with more
   than 4GHz.  */
static hp_timing_t freq;


/* This function is defined in the thread library.  */
extern void __pthread_clock_settime (clockid_t clock_id, hp_timing_t offset)
     __attribute__ ((__weak__));


static int
hp_timing_settime (clockid_t clock_id, const struct timespec *tp)
{
  hp_timing_t tsc;
  hp_timing_t usertime;

  /* First thing is to get the current time.  */
  HP_TIMING_NOW (tsc);

  if (__builtin_expect (freq == 0, 0))
    {
      /* This can only happen if we haven't initialized the `freq'
	 variable yet.  Do this now. We don't have to protect this
	 code against multiple execution since all of them should lead
	 to the same result.  */
      freq = __get_clockfreq ();
      if (__builtin_expect (freq == 0, 0))
	/* Something went wrong.  */
	return -1;
    }

  /* Convert the user-provided time into CPU ticks.  */
  usertime = tp->tv_sec * freq + (tp->tv_nsec * freq) / 1000000000ull;

  /* Determine the offset and use it as the new base value.  */
  if (clock_id == CLOCK_PROCESS_CPUTIME_ID
      || __pthread_clock_settime == NULL)
    GL(dl_cpuclock_offset) = tsc - usertime;
  else
    __pthread_clock_settime (clock_id, tsc - usertime);

  return 0;
}
#endif


/* Set CLOCK to value TP.  */
int
clock_settime (clockid_t clock_id, const struct timespec *tp)
{
  int retval;

  /* Make sure the time cvalue is OK.  */
  if (tp->tv_nsec < 0 || tp->tv_nsec >= 1000000000)
    {
      __set_errno (EINVAL);
      return -1;
    }

  switch (clock_id)
    {
#define HANDLE_REALTIME \
      do {								      \
	struct timeval tv;						      \
	TIMESPEC_TO_TIMEVAL (&tv, tp);					      \
									      \
	retval = settimeofday (&tv, NULL);				      \
      } while (0)

#ifdef SYSDEP_SETTIME
      SYSDEP_SETTIME;
#endif

#ifndef HANDLED_REALTIME
    case CLOCK_REALTIME:
      HANDLE_REALTIME;
      break;
#endif

    default:
#ifdef SYSDEP_SETTIME_CPU
      SYSDEP_SETTIME_CPU;
#endif
#ifndef HANDLED_CPUTIME
# if HP_TIMING_AVAIL
      if (CPUCLOCK_WHICH (clock_id) == CLOCK_PROCESS_CPUTIME_ID
	  || CPUCLOCK_WHICH (clock_id) == CLOCK_THREAD_CPUTIME_ID)
	retval = hp_timing_settime (clock_id, tp);
      else
# endif
	{
	  __set_errno (EINVAL);
	  retval = -1;
	}
#endif
      break;
    }

  return retval;
}
