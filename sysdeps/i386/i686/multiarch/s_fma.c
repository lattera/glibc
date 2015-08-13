/* Multiple versions of fma.
   Copyright (C) 2010-2015 Free Software Foundation, Inc.
   Contributed by Intel Corporation.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <config.h>

#ifdef HAVE_AVX_SUPPORT
#include <math.h>
#include <init-arch.h>

extern double __fma_ia32 (double x, double y, double z) attribute_hidden;
extern double __fma_fma (double x, double y, double z) attribute_hidden;

libm_ifunc (__fma,
	    HAS_ARCH_FEATURE (FMA_Usable) ? __fma_fma : __fma_ia32);
weak_alias (__fma, fma)

# define __fma __fma_ia32
#endif

#include <sysdeps/ieee754/ldbl-96/s_fma.c>
