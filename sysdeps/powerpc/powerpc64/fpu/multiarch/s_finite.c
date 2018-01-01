/* Multiple versions of finite.
   Copyright (C) 2013-2018 Free Software Foundation, Inc.
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

#define __finite __redirect___finite

/* The following definitions, although not related to the 'double'
   version of 'finite', are required to guarantee macro expansions
   (e.g.: from __finitef to __redirect_finitef) in include/math.h, thus
   compensating for the unintended macro expansions in
   math/bits/mathcalls-helper-functions.h.  */
#define __finitef __redirect___finitef
#define __finitel __redirect___finitel
#define __finitef128 __redirect___finitef128

#include <math.h>
#include <math_ldbl_opt.h>
#include <shlib-compat.h>
#include "init-arch.h"

extern __typeof (__finite) __finite_ppc64 attribute_hidden;
extern __typeof (__finite) __finite_power7 attribute_hidden;
extern __typeof (__finite) __finite_power8 attribute_hidden;
#undef __finite
#undef __finitef
#undef __finitel
#undef __finitef128

libc_ifunc_redirected (__redirect___finite, __finite,
		       (hwcap2 & PPC_FEATURE2_ARCH_2_07)
		       ? __finite_power8
		       : (hwcap & PPC_FEATURE_ARCH_2_06)
			 ? __finite_power7
			 : __finite_ppc64);

weak_alias (__finite, finite)

#ifdef NO_LONG_DOUBLE
strong_alias (__finite, __finitel)
weak_alias (__finite, finitel)
#endif

#if IS_IN (libm)
# if LONG_DOUBLE_COMPAT (libm, GLIBC_2_0)
compat_symbol (libm, finite, finitel, GLIBC_2_0);
# endif
# if LONG_DOUBLE_COMPAT (libm, GLIBC_2_1)
compat_symbol (libm, __finite, __finitel, GLIBC_2_1);
# endif
#else
# if LONG_DOUBLE_COMPAT (libc, GLIBC_2_0)
compat_symbol (libc, __finite, __finitel, GLIBC_2_0);
compat_symbol (libc, finite, finitel, GLIBC_2_0);
# endif
#endif
