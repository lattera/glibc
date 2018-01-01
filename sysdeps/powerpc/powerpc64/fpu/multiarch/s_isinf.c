/* Multiple versions of isinf.
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

#define __isinf __redirect___isinf

/* The following definitions, although not related to the 'double'
   version of 'isinf', are required to guarantee macro expansions
   (e.g.: from __isinff to __redirect_isinff) in include/math.h, thus
   compensating for the unintended macro expansions in
   math/bits/mathcalls-helper-functions.h.  */
#define __isinff __redirect___isinff
#define __isinfl __redirect___isinfl
#define __isinff128 __redirect___isinff128

#include <math.h>
#include <math_ldbl_opt.h>
#include <shlib-compat.h>
#include "init-arch.h"

extern __typeof (__isinf) __isinf_ppc64 attribute_hidden;
extern __typeof (__isinf) __isinf_power7 attribute_hidden;
extern __typeof (__isinf) __isinf_power8 attribute_hidden;
#undef __isinf
#undef __isinff
#undef __isinfl
#undef __isinff128

libc_ifunc_redirected (__redirect___isinf, __isinf,
		       (hwcap2 & PPC_FEATURE2_ARCH_2_07)
		       ? __isinf_power8
		       : (hwcap & PPC_FEATURE_ARCH_2_06)
			 ? __isinf_power7
			 : __isinf_ppc64);

weak_alias (__isinf, isinf)

#ifdef NO_LONG_DOUBLE
strong_alias (__isinf, __isinfl)
weak_alias (__isinf, isinfl)
#endif

#if !IS_IN (libm)
# if LONG_DOUBLE_COMPAT (libc, GLIBC_2_0)
compat_symbol (libc, __isinf, __isinfl, GLIBC_2_0);
compat_symbol (libc, isinf, isinfl, GLIBC_2_0);
# endif
#endif
