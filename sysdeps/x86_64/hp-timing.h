/* High precision, low overhead timing functions.  x86-64 version.
   Copyright (C) 2002, 2004 Free Software Foundation, Inc.
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

#ifndef _HP_TIMING_H

/* We can use some of the i686 implementation without changes.  */
# include <sysdeps/i386/i686/hp-timing.h>

/* The "=A" constraint used in 32-bit mode does not work in 64-bit mode.  */
# undef HP_TIMING_NOW
# define HP_TIMING_NOW(Var) \
  ({ unsigned int _hi, _lo; \
     asm volatile ("rdtsc" : "=a" (_lo), "=d" (_hi)); \
     (Var) = ((unsigned long long int) _hi << 32) | _lo; })

/* The funny business for 32-bit mode is not required here.  */
# undef HP_TIMING_ACCUM
# define HP_TIMING_ACCUM(Sum, Diff) ((Sum) += (Diff))


#endif /* hp-timing.h */
