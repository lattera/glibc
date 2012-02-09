/* FMA version of fmaf.
   Copyright (C) 2009, 2010, 2011 Free Software Foundation, Inc.
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
#include <math.h>
#include <init-arch.h>

#ifdef HAVE_AVX_SUPPORT

extern float __fmaf_sse2 (float x, float y, float z) attribute_hidden;


static float
__fmaf_fma3 (float x, float y, float z)
{
  asm ("vfmadd213ss %3, %2, %0" : "=x" (x) : "0" (x), "x" (y), "xm" (z));
  return x;
}


# ifdef HAVE_FMA4_SUPPORT
static float
__fmaf_fma4 (float x, float y, float z)
{
  asm ("vfmaddss %3, %2, %1, %0" : "=x" (x) : "x" (x), "xm" (y), "xm" (z));
  return x;
}
# else
#  undef HAS_FMA4
#  define HAS_FMA4 0
#  define __fmaf_fma4 ((void *) 0)
# endif


libm_ifunc (__fmaf, HAS_FMA
	    ? __fmaf_fma3 : (HAS_FMA4 ? __fmaf_fma4 : __fmaf_sse2));
weak_alias (__fmaf, fmaf)

# define __fmaf __fmaf_sse2
#endif

#include <sysdeps/ieee754/dbl-64/s_fmaf.c>
