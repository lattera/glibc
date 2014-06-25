/* High precision, low overhead timing functions.  IA-64 version.
   Copyright (C) 2001-2014 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 2001.

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

#ifndef _HP_TIMING_H
#define _HP_TIMING_H	1

#include <string.h>
#include <sys/param.h>
#include <_itoa.h>
#include <ia64intrin.h>

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

   - HP_TIMING_NOW: place timestamp for current time in variable given as
     parameter.

   - HP_TIMING_DIFF: compute difference between two times and store it
     in a third.  Source and destination might overlap.

   - HP_TIMING_ACCUM_NT: add time difference to another variable, without
     being thread-safe.

   - HP_TIMING_PRINT: write decimal representation of the timing value into
     the given string.  This operation need not be inline even though
     HP_TIMING_INLINE is specified.

*/

/* We always assume having the timestamp register.  */
#define HP_TIMING_AVAIL		(1)

/* We indeed have inlined functions.  */
#define HP_TIMING_INLINE	(1)

/* We use 64bit values for the times.  */
typedef unsigned long int hp_timing_t;

/* The Itanium/Merced has a bug where the ar.itc register value read
   is not correct in some situations.  The solution is to read again.
   For now we always do this until we know how to recognize a fixed
   processor implementation.  */
#define REPEAT_READ(val) __builtin_expect ((long int) val == -1, 0)

/* That's quite simple.  Use the `ar.itc' instruction.  */
#define HP_TIMING_NOW(Var) \
  ({ unsigned long int __itc;						      \
     do									      \
       asm volatile ("mov %0=ar.itc" : "=r" (__itc) : : "memory");	      \
     while (REPEAT_READ (__itc));					      \
     Var = __itc; })

/* It's simple arithmetic for us.  */
#define HP_TIMING_DIFF(Diff, Start, End)	(Diff) = ((End) - (Start))

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
    memcpy (__dest, " clock cycles", MIN (__len,			      \
					  (int) sizeof (" clock cycles")));   \
  } while (0)

#endif	/* hp-timing.h */
