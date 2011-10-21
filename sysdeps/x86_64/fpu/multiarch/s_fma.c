/* FMA version of fma.
   Copyright (C) 2009, 2010, 2011 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA. */

#include <config.h>
#include <math.h>
#include <init-arch.h>

#ifdef HAVE_AVX_SUPPORT

extern double __fma_sse2 (double x, double y, double z) attribute_hidden;


static double
__fma_fma3 (double x, double y, double z)
{
  asm ("vfmadd213sd %3, %2, %0" : "=x" (x) : "0" (x), "x" (y), "xm" (z));
  return x;
}


# ifdef HAVE_FMA4_SUPPORT
static double
__fma_fma4 (double x, double y, double z)
{
  asm ("vfmaddsd %3, %2, %1, %0" : "=x" (x) : "x" (x), "xm" (y), "xm" (z));
  return x;
}
# else
#  undef HAS_FMA4
#  define HAS_FMA4 0
#  define __fma_fma4 ((void *) 0)
# endif


libm_ifunc (__fma, HAS_FMA
	    ? __fma_fma3 : (HAS_FMA4 ? __fma_fma4 : __fma_sse2));
weak_alias (__fma, fma)

# define __fma __fma_sse2
#endif

#include <sysdeps/ieee754/dbl-64/s_fma.c>
