/* Define timing macros.
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

#include <hp-timing.h>
#include <stdint.h>

#if HP_TIMING_AVAIL && !defined USE_CLOCK_GETTIME
# define GL(x) _##x
# define GLRO(x) _##x
hp_timing_t _dl_hp_timing_overhead;
typedef hp_timing_t timing_t;

# define TIMING_INIT(iters) \
({									      \
  HP_TIMING_DIFF_INIT();						      \
  (iters) = 1000;							      \
})

# define TIMING_NOW(var) HP_TIMING_NOW (var)
# define TIMING_DIFF(diff, start, end) HP_TIMING_DIFF ((diff), (start), (end))
# define TIMING_ACCUM(sum, diff) HP_TIMING_ACCUM_NT ((sum), (diff))

# define TIMING_PRINT_STATS(func, d_total_s, d_iters, d_total_i, max, min) \
  printf ("%s: ITERS:%g: TOTAL:%gMcy, MAX:%gcy, MIN:%gcy, %g calls/Mcy\n",    \
	  (func), (d_total_i), (d_total_s) * 1e-6, (max) / (d_iters),	      \
	  (min) / (d_iters), 1e6 * (d_total_i) / (d_total_s));

#else
typedef uint64_t timing_t;

/* Measure 1000 times the resolution of the clock.  So for a 1ns
   resolution  clock, we measure 1000 iterations of the function call at a
   time.  Measurements close to the minimum clock resolution won't make
   much sense, but it's better than having nothing at all.  */
# define TIMING_INIT(iters) \
({									      \
  struct timespec start;						      \
  clock_getres (CLOCK_PROCESS_CPUTIME_ID, &start);			      \
  (iters) = 1000 * start.tv_nsec;					      \
})

# define TIMING_NOW(var) \
({									      \
  struct timespec tv;							      \
  clock_gettime (CLOCK_PROCESS_CPUTIME_ID, &tv);			      \
  (var) = (uint64_t) (tv.tv_nsec + (uint64_t) 1000000000 * tv.tv_sec);	      \
})

# define TIMING_DIFF(diff, start, end) (diff) = (end) - (start)
# define TIMING_ACCUM(sum, diff) (sum) += (diff)

# define TIMING_PRINT_STATS(func, d_total_s, d_iters, d_total_i, max, min) \
  printf ("%s: ITERS:%g: TOTAL:%gs, MAX:%gns, MIN:%gns, %g iter/s\n",	      \
	  (func), (d_total_i), (d_total_s) * 1e-9, (max) / (d_iters),		      \
	  (min) / (d_iters), 1e9 * (d_total_i) / (d_total_s))

#endif
