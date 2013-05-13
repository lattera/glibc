/* Skeleton for benchmark programs.
   Copyright (C) 2013 Free Software Foundation, Inc.
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

#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <inttypes.h>
#include "bench-timing.h"

volatile unsigned int dontoptimize = 0;

void
startup (void)
{
  /* This loop should cause CPU to switch to maximal freqency.
     This makes subsequent measurement more accurate.  We need a side effect
     to prevent the loop being deleted by compiler.
     This should be enough to cause CPU to speed up and it is simpler than
     running loop for constant time. This is used when user does not have root
     access to set a constant freqency.  */
  for (int k = 0; k < 10000000; k++)
    dontoptimize += 23 * dontoptimize + 2;
}

#define TIMESPEC_AFTER(a, b) \
  (((a).tv_sec == (b).tv_sec) ?						      \
     ((a).tv_nsec > (b).tv_nsec) :					      \
	((a).tv_sec > (b).tv_sec))
int
main (int argc, char **argv)
{
  unsigned long i, k;
  struct timespec runtime;
  timing_t start, end;

  startup();

  memset (&runtime, 0, sizeof (runtime));

  unsigned long iters;

  TIMING_INIT (iters);

  for (int v = 0; v < NUM_VARIANTS; v++)
    {
      /* Run for approximately DURATION seconds.  */
      clock_gettime (CLOCK_MONOTONIC_RAW, &runtime);
      runtime.tv_sec += DURATION;

      double d_total_i = 0;
      timing_t total = 0, max = 0, min = 0x7fffffffffffffff;
      while (1)
	{
	  for (i = 0; i < NUM_SAMPLES (v); i++)
	    {
	      uint64_t cur;
	      TIMING_NOW (start);
	      for (k = 0; k < iters; k++)
		BENCH_FUNC (v, i);
	      TIMING_NOW (end);

	      TIMING_DIFF (cur, start, end);

	      if (cur > max)
		max = cur;

	      if (cur < min)
		min = cur;

	      TIMING_ACCUM (total, cur);

	      d_total_i += iters;
	    }
	  struct timespec curtime;

	  memset (&curtime, 0, sizeof (curtime));
	  clock_gettime (CLOCK_MONOTONIC_RAW, &curtime);
	  if (TIMESPEC_AFTER (curtime, runtime))
	    goto done;
	}

      double d_total_s;
      double d_iters;

    done:
      d_total_s = total;
      d_iters = iters;

      TIMING_PRINT_STATS (VARIANT (v), d_total_s, d_iters, d_total_i, max,
			  min);
    }

  return 0;
}
