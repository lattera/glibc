/* Multiple versions of finite.
   Copyright (C) 2013-2015 Free Software Foundation, Inc.
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

extern __typeof (__finite) __finite_ppc32 attribute_hidden;
extern __typeof (__finite) __finite_power7 attribute_hidden;

libc_ifunc (__finite,
	    (hwcap & PPC_FEATURE_ARCH_2_06)
	    ? __finite_power7
            : __finite_ppc32);

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
