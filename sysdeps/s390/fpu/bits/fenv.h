/* Copyright (C) 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Denis Joseph Barrow (djbarrow@de.ibm.com).

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

#ifndef _FENV_H
# error "Never use <bits/fenv.h> directly; include <fenv.h> instead."
#endif

/* Define bits representing the exception.  We use the bit positions
   of the appropriate bits in the FPU control word.  */
enum
  {
    FE_INVALID = 0x80,
#define FE_INVALID	FE_INVALID
    FE_DIVBYZERO = 0x40,
#define FE_DIVBYZERO	FE_DIVBYZERO
    FE_OVERFLOW = 0x20,
#define FE_OVERFLOW	FE_OVERFLOW
    FE_UNDERFLOW = 0x10,
#define FE_UNDERFLOW	FE_UNDERFLOW
    FE_INEXACT = 0x08
#define FE_INEXACT	FE_INEXACT
  };
/* We dont use the y bit of the DXC in the floating point control register
   as glibc has no FE encoding for fe inexact incremented
   or fe inexact truncated.
   We currently  use the flag bits in the fpc
   as these are sticky for feholdenv & feupdatenv as it is defined
   in the HP Manpages.  */


#define FE_ALL_EXCEPT \
	(FE_INEXACT | FE_DIVBYZERO | FE_UNDERFLOW | FE_OVERFLOW | FE_INVALID)

enum
  {
    FE_TONEAREST = 0,
#define FE_TONEAREST	FE_TONEAREST
    FE_DOWNWARD = 0x3,
#define FE_DOWNWARD	FE_DOWNWARD
    FE_UPWARD = 0x2,
#define FE_UPWARD	FE_UPWARD
    FE_TOWARDZERO = 0x1
#define FE_TOWARDZERO	FE_TOWARDZERO
  };


/* Type representing exception flags.  */
typedef unsigned int fexcept_t; /* size of fpc */


/* Type representing floating-point environment.  This function corresponds
   to the layout of the block written by the `fstenv'.  */
typedef struct
{
  fexcept_t fpc;
  void *ieee_instruction_pointer;
  /* failing instruction for ieee exceptions */
} fenv_t;

/* If the default argument is used we use this value.  */
#define FE_DFL_ENV	((__const fenv_t *) -1)

#ifdef __USE_GNU
/* Floating-point environment where none of the exceptions are masked.  */
# define FE_NOMASK_ENV	((__const fenv_t *) -2)
#endif
