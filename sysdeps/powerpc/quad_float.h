/* Internal libc stuff for 128-bit IEEE FP emulation routines.
   Copyright (C) 1997 Free Software Foundation, Inc.
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

#ifndef _QUAD_FLOAT_H
#define _QUAD_FLOAT_H	1

#include <fenv_libc.h>

/* Returns the number of leading zero bits in 'x' (between 0 and 32
   inclusive).  'x' is treated as being 32 bits long.  */
#define cntlzw(x) \
        ({ unsigned p = (x); \
	   unsigned r; \
	   asm ("cntlzw %0,%1" : "=r"(r) : "r"(p)); \
	   r; })

/* Returns the number of leading zero bits in 'x' (between 0 and 64
   inclusive).  'x' is treated as being 64 bits long.  */
#define cntlzd(x) \
        ({ unsigned long long q = (x); \
	   unsigned int c1, c2; \
	   c1 = cntlzw(q >> 32); \
	   c2 = cntlzw(q); \
	   c1 + (-(c1 >> 5) & c2); })

#define shift_and_or
 
#endif /* quad_float.h */
