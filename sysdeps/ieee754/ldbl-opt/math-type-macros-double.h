/* Overrides for ldbl-opt versioning for double types.
   Copyright (C) 2016-2017 Free Software Foundation, Inc.
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

#ifndef _MATH_TYPE_MACROS_DOUBLE

#include <math_ldbl_opt.h>
#include <first-versions.h>

/* Define compat symbols for long double on platforms
   where it was not always a distinct type.  */
#if !defined M_LIBM_NEED_COMPAT
# define M_LIBM_NEED_COMPAT(f) \
  LONG_DOUBLE_COMPAT (libm, FIRST_VERSION_libm_ ## f ## l)
#endif

#if !defined declare_mgen_libm_compat
# define declare_mgen_libm_compat(from, to)	      \
  compat_symbol (libm, from, to ## l,		      \
		 FIRST_VERSION_libm_ ## to ## l);
#endif

#include_next <math-type-macros-double.h>
#endif
