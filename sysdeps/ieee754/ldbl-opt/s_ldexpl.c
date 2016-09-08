/* ldexpl alias overrides for platforms where long double
   was previously not unique.
   Copyright (C) 2016 Free Software Foundation, Inc.
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

#define declare_mgen_alias(f,t)
#include <math-type-macros-ldouble.h>
#include <s_ldexp_template.c>

strong_alias (__ldexpl, __ldexpl_2)
#if IS_IN (libm)
long_double_symbol (libm, __ldexpl, ldexpl);
long_double_symbol (libm, __ldexpl_2, scalbnl);
#else
long_double_symbol (libc, __ldexpl, ldexpl);
long_double_symbol (libc, __ldexpl_2, scalbnl);
#endif
