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

int
main (int argc, char **argv)
{
  unsigned long i, j, k;
  uint64_t total = 0, max = 0, min = 0x7fffffffffffffff;
  struct timespec start, end;

  memset (&start, 0, sizeof (start));
  memset (&end, 0, sizeof (end));

  clock_getres (CLOCK_PROCESS_CPUTIME_ID, &start);

  /* Measure 1000 times the resolution of the clock.  So for a 1ns resolution
     clock, we measure 1000 iterations of the function call at a time.
     Measurements close to the minimum clock resolution won't make much sense,
     but it's better than having nothing at all.  */
  unsigned long iters = 1000 * start.tv_nsec;
  unsigned long total_iters = ITER / iters;

  for (i = 0; i < NUM_SAMPLES; i++)
    {
      for (j = 0; j < total_iters; j ++)
	{
	  clock_gettime (CLOCK_PROCESS_CPUTIME_ID, &start);
	  for (k = 0; k < iters; k++)
	    BENCH_FUNC(i);
	  clock_gettime (CLOCK_PROCESS_CPUTIME_ID, &end);

	  uint64_t cur = (end.tv_nsec - start.tv_nsec
			 + ((end.tv_sec - start.tv_sec)
			    * (uint64_t) 1000000000));

	  if (cur > max)
	    max = cur;

	  if (cur < min)
	    min = cur;

	  total += cur;
	}
    }

  double d_total_s = total * 1e-9;
  double d_iters = iters;
  double d_total_i = (double)ITER * NUM_SAMPLES;
  printf (FUNCNAME ": ITERS:%g: TOTAL:%gs, MAX:%gns, MIN:%gns, %g iter/s\n",
	  d_total_i, d_total_s, max / d_iters, min / d_iters,
	  d_total_i / d_total_s);

  return 0;
}
