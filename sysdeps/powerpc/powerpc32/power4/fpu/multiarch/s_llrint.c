/* Multiple versions of llrint.
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

extern __typeof (__llrint) __llrint_ppc32 attribute_hidden;
extern __typeof (__llrint) __llrint_power6 attribute_hidden;

libc_ifunc (__llrint,
	    (hwcap & PPC_FEATURE_ARCH_2_05)
	    ? __llrint_power6
            : __llrint_ppc32);

weak_alias (__llrint, llrint)

#ifdef NO_LONG_DOUBLE
strong_alias (__llrint, __llrintl)
weak_alias (__llrint, llrintl)
#endif
#if LONG_DOUBLE_COMPAT(libm, GLIBC_2_1)
compat_symbol (libm, __llrint, llrintl, GLIBC_2_1);
#endif
