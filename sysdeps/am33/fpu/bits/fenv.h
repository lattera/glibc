/* Copyright (C) 1998, 1999, 2000, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Alexandre Oliva <aoliva@redhat.com>
   based on the corresponding file in the mips port.

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


/* Define bits representing the exception.  We use the EF bit
   positions of the appropriate bits in the FPCR register.  */
enum
  {
    FE_INEXACT = 0x01,
#define FE_INEXACT	FE_INEXACT
    FE_UNDERFLOW = 0x02,
#define FE_UNDERFLOW	FE_UNDERFLOW
    FE_OVERFLOW = 0x04,
#define FE_OVERFLOW	FE_OVERFLOW
    FE_DIVBYZERO = 0x08,
#define FE_DIVBYZERO	FE_DIVBYZERO
    FE_INVALID = 0x10,
#define FE_INVALID	FE_INVALID
  };

#define FE_ALL_EXCEPT \
	(FE_INEXACT | FE_DIVBYZERO | FE_UNDERFLOW | FE_OVERFLOW | FE_INVALID)

/* The AM33/2.0 FPU supports only Round to nearest.  Bits 3<<16 are
   reserved to represent other rounding modes.  */
enum
  {
    FE_TONEAREST = 0x00000,
#define FE_TONEAREST	FE_TONEAREST
  };


/* Type representing exception flags.  */
typedef unsigned int fexcept_t;


/* Type representing floating-point environment.  */
typedef unsigned int fenv_t;

/* If the default argument is used we use this value.  */
#define FE_DFL_ENV	((__const fenv_t *) -1)

#ifdef __USE_GNU
/* Floating-point environment where none of the exception is masked.  */
# define FE_NOMASK_ENV  ((__const fenv_t *) -2)
#endif
