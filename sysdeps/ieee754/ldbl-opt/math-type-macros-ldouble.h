/* Overrides for ldbl-opt versioning for long double types.
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

#ifndef _MATH_TYPE_MACROS_LDOUBLE

#include <math_ldbl_opt.h>

/* Use properly versioned symbols for long double on platforms where
   it was not always a distinct type.  */
#if !defined declare_mgen_alias
# define declare_mgen_alias(from, to) \
  long_double_symbol (libm, from ## l, to ## l);
#endif

#include_next <math-type-macros-ldouble.h>
#endif
