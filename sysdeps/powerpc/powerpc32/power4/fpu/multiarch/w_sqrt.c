/* Multiple versions of sqrt.
   Copyright (C) 2013-2016 Free Software Foundation, Inc.
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

#include <math.h>
#include <math_ldbl_opt.h>
#include <shlib-compat.h>
#include "init-arch.h"

extern __typeof (__sqrt) __sqrt_ppc32 attribute_hidden;
extern __typeof (__sqrt) __sqrt_power5 attribute_hidden;

libc_ifunc (__sqrt,
	    (hwcap & PPC_FEATURE_POWER5)
	    ? __sqrt_power5
	    : __sqrt_ppc32);

weak_alias (__sqrt, sqrt)

#ifdef NO_LONG_DOUBLE
strong_alias (__sqrt, __sqrtl)
weak_alias (__sqrt, sqrtl)
#endif
#if LONG_DOUBLE_COMPAT(libm, GLIBC_2_0)
compat_symbol (libm, __sqrt, sqrtl, GLIBC_2_0);
#endif
