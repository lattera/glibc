/* Copyright (C) 1997, 1998, 1999, 2000, 2012 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef _FENV_H
# error "Never use <bits/fenv.h> directly; include <fenv.h> instead."
#endif


/* Define bits representing the exception.  We use the bit positions of
   the appropriate bits in the FPSR Accrued Exception Byte.  */
enum
  {
    FE_INEXACT =
#define FE_INEXACT	(1 << 3)
      FE_INEXACT,
    FE_DIVBYZERO =
#define FE_DIVBYZERO	(1 << 4)
      FE_DIVBYZERO,
    FE_UNDERFLOW =
#define FE_UNDERFLOW	(1 << 5)
      FE_UNDERFLOW,
    FE_OVERFLOW =
#define FE_OVERFLOW	(1 << 6)
      FE_OVERFLOW,
    FE_INVALID =
#define FE_INVALID	(1 << 7)
      FE_INVALID
  };

#define FE_ALL_EXCEPT \
	(FE_INEXACT | FE_DIVBYZERO | FE_UNDERFLOW | FE_OVERFLOW | FE_INVALID)

/* The m68k FPU supports all of the four defined rounding modes.  We use
   the bit positions in the FPCR Mode Control Byte as the values for the
   appropriate macros.  */
enum
  {
    FE_TONEAREST =
#define FE_TONEAREST	0
      FE_TONEAREST,
    FE_TOWARDZERO =
#define FE_TOWARDZERO	(1 << 4)
      FE_TOWARDZERO,
    FE_DOWNWARD =
#define FE_DOWNWARD	(2 << 4)
      FE_DOWNWARD,
    FE_UPWARD =
#define FE_UPWARD	(3 << 4)
      FE_UPWARD
  };


/* Type representing exception flags.  */
typedef unsigned int fexcept_t;


/* Type representing floating-point environment.  This structure
   corresponds to the layout of the block written by `fmovem'.  */
typedef struct
  {
    unsigned int __control_register;
    unsigned int __status_register;
    unsigned int __instruction_address;
  }
fenv_t;

/* If the default argument is used we use this value.  */
#define FE_DFL_ENV	((const fenv_t *) -1)

#ifdef __USE_GNU
/* Floating-point environment where none of the exceptions are masked.  */
# define FE_NOMASK_ENV	((const fenv_t *) -2)
#endif
