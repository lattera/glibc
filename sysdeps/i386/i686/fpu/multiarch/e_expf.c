/* Multiple versions of expf.
   Copyright (C) 2012-2018 Free Software Foundation, Inc.
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

extern float __redirect_expf (float);

#define SYMBOL_NAME expf
#include "ifunc-sse2.h"

libc_ifunc_redirected (__redirect_expf, __expf, IFUNC_SELECTOR ());

#include <libm-alias-float.h>
#ifdef SHARED
__hidden_ver1 (__expf_ia32, __GI___expf, __redirect_expf)
  __attribute__ ((visibility ("hidden")));

# include <shlib-compat.h>
versioned_symbol (libm, __expf, expf, GLIBC_2_27);
libm_alias_float_other (__exp, exp)
#else
libm_alias_float (__exp, exp)
#endif

strong_alias (__expf, __ieee754_expf)
strong_alias (__expf, __expf_finite)

#define __expf __expf_ia32
#include <sysdeps/ieee754/flt-32/e_expf.c>
