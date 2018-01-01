/* Multiple versions of logf.
   Copyright (C) 2017-2018 Free Software Foundation, Inc.
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

#include <libm-alias-float.h>

extern float __redirect_logf (float);

#define SYMBOL_NAME logf
#include "ifunc-fma.h"

libc_ifunc_redirected (__redirect_logf, __logf, IFUNC_SELECTOR ());

#ifdef SHARED
__hidden_ver1 (__logf, __GI___logf, __redirect_logf)
  __attribute__ ((visibility ("hidden")));

# include <shlib-compat.h>
versioned_symbol (libm, __logf, logf, GLIBC_2_27);
libm_alias_float_other (__log, log)
#else
libm_alias_float (__log, log)
#endif

strong_alias (__logf, __ieee754_logf)
strong_alias (__logf, __logf_finite)

#define __logf __logf_sse2
#include <sysdeps/ieee754/flt-32/e_logf.c>
