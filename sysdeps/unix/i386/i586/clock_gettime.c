/* Copyright (C) 1999, 2000 Free Software Foundation, Inc.
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

#include <sys/time.h>
#include <libc-internal.h>

/* This implementation uses the TSC register in modern (i586 and up) IA-32
   processors (most modern clones also provide it).  Since we need the
   resolution of the clock and since determining this is not cheap, we
   cache the value.  But this means that systems with processors running
   at different speeds or process migration to machines with slower or
   faster processors will not work without changes.  */


/* Clock frequency of the processor.  We make it a 64-bit variable
   because some jokers are already playing with processors with more
   than 4GHz.  */
static unsigned long long int freq;


/* We add an limitation here: we assume that the machine is not up as
   long as it takes to wrap-around the 64-bit timestamp counter.  On a
   4GHz machine it would take 136 years of uptime to wrap around so
   this "limitation" is not severe.

   We use this clock also as the monotonic clock since we don't allow
   setting the CPU-time clock.  If this should ever change we will have
   to separate the two.  */
#define EXTRA_CLOCK_CASES \
  case CLOCK_PROCESS_CPUTIME_ID:					      \
  case CLOCK_THREAD_CPUTIME_ID:						      \
    {									      \
      unsigned long long int tsc;					      \
									      \
      if (__builtin_expect (freq == 0, 0))				      \
	{								      \
	  /* This can only happen if we haven't initialized the `freq'	      \
	     variable yet.  Do this now. We don't have to protect this	      \
	     code against multiple execution since all of them should	      \
	     lead to the same result.  */				      \
	  freq = __get_clockfreq ();					      \
	  if (__builtin_expect (freq == 0, 0))				      \
	    /* Something went wrong.  */				      \
	    break;							      \
	}								      \
									      \
      /* Get the current counter.  */					      \
      asm volatile ("rdtsc" : "=A" (tsc));				      \
									      \
      /* Compute the seconds.  */					      \
      tp->tv_sec = tsc / freq;						      \
									      \
      /* And the nanoseconds.  This computation should be stable until	      \
	 we get machines with about 16GHz frequency.  */		      \
      tp->tv_nsec = ((tsc % freq) * 1000000000ull) / freq;		      \
									      \
      retval = 0;							      \
    }									      \
    break;

#include <sysdeps/unix/clock_gettime.c>
