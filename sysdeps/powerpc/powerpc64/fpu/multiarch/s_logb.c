/* Multiple versions of logb.
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

extern __typeof (__logb) __logb_ppc64 attribute_hidden;
extern __typeof (__logb) __logb_power7 attribute_hidden;

libc_ifunc (__logb,
	    (hwcap & PPC_FEATURE_ARCH_2_06)
	    ? __logb_power7
            : __logb_ppc64);

weak_alias (__logb, logb)

#ifdef NO_LONG_DOUBLE
strong_alias (__logb, __logbl)
weak_alias (__logb, logbl)
#endif

#if LONG_DOUBLE_COMPAT (libm, GLIBC_2_0)
compat_symbol (libm, logb, logbl, GLIBC_2_0);
#endif
