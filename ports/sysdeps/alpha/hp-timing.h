/* High precision, low overhead timing functions.  Alpha version.
   Copyright (C) 2001 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Richard Henderson <rth@redhat.com>, 2001.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef _HP_TIMING_H
#define _HP_TIMING_H	1

#include <string.h>
#include <sys/param.h>
#include <_itoa.h>

/* The macros defined here use the timestamp counter in IA-64.  They
   provide a very accurate way to measure the time with very little
   overhead.  The time values themself have no real meaning, only
   differences are interesting.

   The list of macros we need includes the following:

   - HP_TIMING_AVAIL: test for availability.

   - HP_TIMING_INLINE: this macro is non-zero if the functionality is not
     implemented using function calls but instead uses some inlined code
     which might simply consist of a few assembler instructions.  We have to
     know this since we might want to use the macros here in places where we
     cannot make function calls.

   - hp_timing_t: This is the type for variables used to store the time
     values.

   - HP_TIMING_ZERO: clear `hp_timing_t' object.

   - HP_TIMING_NOW: place timestamp for current time in variable given as
     parameter.

   - HP_TIMING_DIFF_INIT: do whatever is necessary to be able to use the
     HP_TIMING_DIFF macro.

   - HP_TIMING_DIFF: compute difference between two times and store it
     in a third.  Source and destination might overlap.

   - HP_TIMING_ACCUM: add time difference to another variable.  This might
     be a bit more complicated to implement for some platforms as the
     operation should be thread-safe and 64bit arithmetic on 32bit platforms
     is not.

   - HP_TIMING_ACCUM_NT: this is the variant for situations where we know
     there are no threads involved.

   - HP_TIMING_PRINT: write decimal representation of the timing value into
     the given string.  This operation need not be inline even though
     HP_TIMING_INLINE is specified.
*/

/* We always have the timestamp register, but it's got only a 4 second
   range.  Use it for ld.so profiling only.  */
#define HP_TIMING_AVAIL		(0)
#define HP_SMALL_TIMING_AVAIL	(1)

/* We indeed have inlined functions.  */
#define HP_TIMING_INLINE	(1)

/* We use 32 bit values for the times.  */
typedef unsigned int hp_timing_t;

/* Set timestamp value to zero.  */
#define HP_TIMING_ZERO(VAR)	(VAR) = (0)

/* The "rpcc" instruction returns a 32-bit counting half and a 32-bit
   "virtual cycle counter displacement".  Subtracting the two gives us
   a virtual cycle count.  */
#define HP_TIMING_NOW(VAR) \
  do {									      \
    unsigned long int x_;						      \
    asm volatile ("rpcc %0" : "=r"(x_));				      \
    (VAR) = (int) (x_) - (int) (x_ >> 32);				      \
  } while (0)

/* ??? Two rpcc instructions can be scheduled simultaneously.  */
#define HP_TIMING_DIFF_INIT() do { } while (0)

/* It's simple arithmetic for us.  */
#define HP_TIMING_DIFF(Diff, Start, End)	(Diff) = ((End) - (Start))

/* ??? Don't bother, since we're only used for ld.so.  */
#define HP_TIMING_ACCUM(Sum, Diff)  not implemented

/* No threads, no extra work.  */
#define HP_TIMING_ACCUM_NT(Sum, Diff)	(Sum) += (Diff)

/* Print the time value.  */
#define HP_TIMING_PRINT(Buf, Len, Val) \
  do {									      \
    char __buf[20];							      \
    char *__cp = _itoa_word (Val, __buf + sizeof (__buf), 10, 0);	      \
    int __len = (Len);							      \
    char *__dest = (Buf);						      \
    while (__len-- > 0 && __cp < __buf + sizeof (__buf))		      \
      *__dest++ = *__cp++;						      \
    memcpy (__dest, " clock cycles", MIN (__len, sizeof (" clock cycles")));  \
  } while (0)

#endif	/* hp-timing.h */
