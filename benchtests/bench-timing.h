/* Define timing macros.
   Copyright (C) 2013-2018 Free Software Foundation, Inc.
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

#undef attribute_hidden
#define attribute_hidden
#include <hp-timing.h>
#include <stdint.h>

#if HP_TIMING_AVAIL && !defined USE_CLOCK_GETTIME
# define GL(x) _##x
# define GLRO(x) _##x
typedef hp_timing_t timing_t;

# define TIMING_TYPE "hp_timing"

# define TIMING_INIT(res) ({ (res) = 1; })

# define TIMING_NOW(var) HP_TIMING_NOW (var)
# define TIMING_DIFF(diff, start, end) HP_TIMING_DIFF ((diff), (start), (end))
# define TIMING_ACCUM(sum, diff) HP_TIMING_ACCUM_NT ((sum), (diff))

#else

#include <time.h>
typedef uint64_t timing_t;

# define TIMING_TYPE "clock_gettime"

/* Measure the resolution of the clock so we can scale the number of
   benchmark iterations by this value.  */
# define TIMING_INIT(res) \
({									      \
  struct timespec start;						      \
  clock_getres (CLOCK_PROCESS_CPUTIME_ID, &start);			      \
  (res) = start.tv_nsec;					      \
})

# define TIMING_NOW(var) \
({									      \
  struct timespec tv;							      \
  clock_gettime (CLOCK_PROCESS_CPUTIME_ID, &tv);			      \
  (var) = (uint64_t) (tv.tv_nsec + (uint64_t) 1000000000 * tv.tv_sec);	      \
})

# define TIMING_DIFF(diff, start, end) (diff) = (end) - (start)
# define TIMING_ACCUM(sum, diff) (sum) += (diff)

#endif

#define TIMING_PRINT_MEAN(d_total_s, d_iters) \
  printf ("\t%g", (d_total_s) / (d_iters))
