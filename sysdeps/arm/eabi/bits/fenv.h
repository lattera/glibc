/* Copyright (C) 2004, 2005 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GCC; see the file COPYING.  If not, write to the Free
   Software Foundation, 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.  */

#ifndef _FENV_H
# error "Never use <bits/fenv.h> directly; include <fenv.h> instead."
#endif

/* Define bits representing exceptions in the FPU status word.  */
enum
  {
    FE_INVALID = 1,
#define FE_INVALID FE_INVALID
    FE_DIVBYZERO = 2,
#define FE_DIVBYZERO FE_DIVBYZERO
    FE_OVERFLOW = 4,
#define FE_OVERFLOW FE_OVERFLOW
    FE_UNDERFLOW = 8,
#define FE_UNDERFLOW FE_UNDERFLOW
    FE_INEXACT = 16,
#define FE_INEXACT FE_INEXACT
  };

/* Amount to shift by to convert an exception to a mask bit.  */
#define FE_EXCEPT_SHIFT	8

/* All supported exceptions.  */
#define FE_ALL_EXCEPT	\
	(FE_INVALID | FE_DIVBYZERO | FE_OVERFLOW | FE_UNDERFLOW | FE_INEXACT)

/* VFP supports all of the four defined rounding modes.  */
enum
  {
    FE_TONEAREST = 0,
#define FE_TONEAREST FE_TONEAREST
    FE_UPWARD = 0x400000,
#define FE_UPWARD FE_UPWARD
    FE_DOWNWARD = 0x800000,
#define FE_DOWNWARD FE_DOWNWARD
    FE_TOWARDZERO = 0xc00000
#define FE_TOWARDZERO FE_TOWARDZERO
  };

/* Type representing exception flags. */
typedef unsigned int fexcept_t;

/* Type representing floating-point environment.  */
typedef struct
  {
    unsigned int __cw;
  }
fenv_t;

/* If the default argument is used we use this value.  */
#define FE_DFL_ENV	((fenv_t *) -1l)

#ifdef __USE_GNU
/* Floating-point environment where none of the exceptions are masked.  */
# define FE_NOMASK_ENV  ((__const fenv_t *) -2)
#endif
