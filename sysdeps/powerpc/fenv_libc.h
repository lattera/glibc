/* Internal libc stuff for floating point environment routines.
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

#ifndef _FENV_LIBC_H
#define _FENV_LIBC_H	1

#include <fenv.h>

/* Transform a logical or of the FE_* bits to a bit pattern for the
   appropriate sticky bits in the FPSCR.  */
#define FE_to_sticky(excepts) \
        (-(excepts & FE_INVALID) & FE_ALL_INVALID \
	 |  (excepts) & (FE_ALL_EXCEPT & ~FE_INVALID  | FE_ALL_INVALID))

/* The sticky bits in the FPSCR indicating exceptions have occurred.  */
#define FPSCR_STICKY_BITS ((FE_ALL_EXCEPT | FE_ALL_INVALID) & ~FE_INVALID)

/* Equivalent to fegetenv, but returns a fenv_t instead of taking a
   pointer.  */
#define fegetenv_register() \
        ({ fenv_t env; asm ("mffs %0" : "=f" (env)); env; })

/* Equivalent to fesetenv, but takes a fenv_t instead of a pointer.  */
#define fesetenv_register(env) \
        ({ double d = (env); asm ("mtfsf 0xff,%0" : : "f" (d)); })

/* This very handy macro:
   - Sets the rounding mode to 'round to nearest';
   - Sets the processor into IEEE mode; and
   - Prevents exceptions from being raised for inexact results.
   These things happen to be exactly what you need for typical elementary
   functions.  */
#define relax_fenv_state() asm ("mtfsfi 7,0")

typedef union
{
  fenv_t fenv;
  unsigned int l[2];
} fenv_union_t;

#endif /* fenv_libc.h */
