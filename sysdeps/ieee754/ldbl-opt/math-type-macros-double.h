/* Overrides for ldbl-opt versioning for double types.
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

#ifndef _MATH_TYPE_MACROS_DOUBLE

#include <math_ldbl_opt.h>

#define LDOUBLE_cabsl_libm_version GLIBC_2_1
#define LDOUBLE_cargl_libm_version GLIBC_2_1
#define LDOUBLE_cimagl_libm_version GLIBC_2_1
#define LDOUBLE_conjl_libm_version GLIBC_2_1
#define LDOUBLE_creall_libm_version GLIBC_2_1
#define LDOUBLE_cacosl_libm_version GLIBC_2_1
#define LDOUBLE_cacoshl_libm_version GLIBC_2_1
#define LDOUBLE_ccosl_libm_version GLIBC_2_1
#define LDOUBLE_ccoshl_libm_version GLIBC_2_1
#define LDOUBLE_casinl_libm_version GLIBC_2_1
#define LDOUBLE_csinl_libm_version GLIBC_2_1
#define LDOUBLE_casinhl_libm_version GLIBC_2_1
#define LDOUBLE_csinhl_libm_version GLIBC_2_1
#define LDOUBLE_catanl_libm_version GLIBC_2_1
#define LDOUBLE_catanhl_libm_version GLIBC_2_1
#define LDOUBLE_ctanl_libm_version GLIBC_2_1
#define LDOUBLE_ctanhl_libm_version GLIBC_2_1
#define LDOUBLE_cexpl_libm_version GLIBC_2_1
#define LDOUBLE_clogl_libm_version GLIBC_2_1
#define LDOUBLE_cprojl_libm_version GLIBC_2_1
#define LDOUBLE_csqrtl_libm_version GLIBC_2_1
#define LDOUBLE_cpowl_libm_version GLIBC_2_1
#define LDOUBLE_clog10l_libm_version GLIBC_2_1
#define LDOUBLE___clog10l_libm_version GLIBC_2_1
#define LDOUBLE_fdiml_libm_version GLIBC_2_1
#define LDOUBLE_fmaxl_libm_version GLIBC_2_1
#define LDOUBLE_fminl_libm_version GLIBC_2_1
#define LDOUBLE_ilogbl_libm_version GLIBC_2_0
#define LDOUBLE_nanl_libm_version GLIBC_2_1

/* Define compat symbols for long double on platforms
   where it was not always a distinct type.  */
#if !defined M_LIBM_NEED_COMPAT
# define M_LIBM_NEED_COMPAT(f) \
  LONG_DOUBLE_COMPAT (libm, LDOUBLE_ ## f ## l_libm_version)
#endif

#if !defined declare_mgen_libm_compat
# define declare_mgen_libm_compat(from, to)	      \
  compat_symbol (libm, from, to ## l,		      \
		 LDOUBLE_ ## to ## l_libm_version);
#endif

#include_next <math-type-macros-double.h>
#endif
