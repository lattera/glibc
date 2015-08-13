/* Multiple versions of expf
   Copyright (C) 2012-2015 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <init-arch.h>

extern double __ieee754_expf_sse2 (double);
extern double __ieee754_expf_ia32 (double);

double __ieee754_expf (double);
libm_ifunc (__ieee754_expf,
	    HAS_CPU_FEATURE (SSE2)
	    ? __ieee754_expf_sse2
	    : __ieee754_expf_ia32);

extern double __expf_finite_sse2 (double);
extern double __expf_finite_ia32 (double);

double __expf_finite (double);
libm_ifunc (__expf_finite,
	    HAS_CPU_FEATURE (SSE2)
	    ? __expf_finite_sse2
	    : __expf_finite_ia32);
