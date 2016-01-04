/* Multiple versions of llround.
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

#define lround __hidden_lround
#define __lround __hidden___lround

#include <math.h>
#include <math_ldbl_opt.h>
#include <shlib-compat.h>
#include "init-arch.h"

extern __typeof (__llround) __llround_ppc64 attribute_hidden;
extern __typeof (__llround) __llround_power5plus attribute_hidden;
extern __typeof (__llround) __llround_power6x attribute_hidden;
extern __typeof (__llround) __llround_power8 attribute_hidden;

libc_ifunc (__llround,
	    (hwcap2 & PPC_FEATURE2_ARCH_2_07)
	    ? __llround_power8 :
	      (hwcap & PPC_FEATURE_POWER6_EXT)
	      ? __llround_power6x :
		(hwcap & PPC_FEATURE_POWER5_PLUS)
		? __llround_power5plus
            : __llround_ppc64);

weak_alias (__llround, llround)

#ifdef NO_LONG_DOUBLE
weak_alias (__llround, llroundl)
strong_alias (__llround, __llroundl)
#endif
#if LONG_DOUBLE_COMPAT(libm, GLIBC_2_1)
compat_symbol (libm, __llround, llroundl, GLIBC_2_1);
compat_symbol (libm, llround, lroundl, GLIBC_2_1);
#endif

/* long has the same width as long long on PPC64.  */
#undef lround
#undef __lround
strong_alias (__llround, __lround)
weak_alias (__llround, lround)
#ifdef NO_LONG_DOUBLE
strong_alias (__llround, __llroundl)
weak_alias (__llround, llroundl)
#endif
#if LONG_DOUBLE_COMPAT(libm, GLIBC_2_1)
compat_symbol (libm, __lround, lroundl, GLIBC_2_1);
#endif
