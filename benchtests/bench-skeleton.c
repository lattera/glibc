/* Skeleton for benchmark programs.
   Copyright (C) 2013-2015 Free Software Foundation, Inc.
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
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <inttypes.h>
#include "bench-timing.h"
#include "json-lib.h"

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
  bool detailed = false;
  json_ctx_t json_ctx;

  if (argc == 2 && !strcmp (argv[1], "-d"))
    detailed = true;

  startup();

  memset (&runtime, 0, sizeof (runtime));

  unsigned long iters, res;

#ifdef BENCH_INIT
  BENCH_INIT ();
#endif
  TIMING_INIT (res);

  iters = 1000 * res;

  json_init (&json_ctx, 2, stdout);

  /* Begin function.  */
  json_attr_object_begin (&json_ctx, FUNCNAME);

  for (int v = 0; v < NUM_VARIANTS; v++)
    {
      /* Run for approximately DURATION seconds.  */
      clock_gettime (CLOCK_MONOTONIC_RAW, &runtime);
      runtime.tv_sec += DURATION;

      double d_total_i = 0;
      timing_t total = 0, max = 0, min = 0x7fffffffffffffff;
      int64_t c = 0;
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
	      /* Accumulate timings for the value.  In the end we will divide
	         by the total iterations.  */
	      RESULT_ACCUM (cur, v, i, c * iters, (c + 1) * iters);

	      d_total_i += iters;
	    }
	  c++;
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

      /* Begin variant.  */
      json_attr_object_begin (&json_ctx, VARIANT (v));

      json_attr_double (&json_ctx, "duration", d_total_s);
      json_attr_double (&json_ctx, "iterations", d_total_i);
      json_attr_double (&json_ctx, "max", max / d_iters);
      json_attr_double (&json_ctx, "min", min / d_iters);
      json_attr_double (&json_ctx, "mean", d_total_s / d_total_i);

      if (detailed)
	{
	  json_array_begin (&json_ctx, "timings");

	  for (int i = 0; i < NUM_SAMPLES (v); i++)
	    json_element_double (&json_ctx, RESULT (v, i));

	  json_array_end (&json_ctx);
	}

      /* End variant.  */
      json_attr_object_end (&json_ctx);
    }

  /* End function.  */
  json_attr_object_end (&json_ctx);

  return 0;
}
