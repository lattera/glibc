/* Helper macros for functions returning a narrower type.
   Copyright (C) 2018 Free Software Foundation, Inc.
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

#ifndef	_MATH_NARROW_H
#define	_MATH_NARROW_H	1

#include <bits/floatn.h>
#include <bits/long-double.h>
#include <errno.h>
#include <fenv.h>
#include <ieee754.h>
#include <math_private.h>

/* Carry out a computation using round-to-odd.  The computation is
   EXPR; the union type in which to store the result is UNION and the
   subfield of the "ieee" field of that union with the low part of the
   mantissa is MANTISSA; SUFFIX is the suffix for the libc_fe* macros
   to ensure that the correct rounding mode is used, for platforms
   with multiple rounding modes where those macros set only the
   relevant mode.  This macro does not work correctly if the sign of
   an exact zero result depends on the rounding mode, so that case
   must be checked for separately.  */
#define ROUND_TO_ODD(EXPR, UNION, SUFFIX, MANTISSA)			\
  ({									\
    fenv_t env;								\
    UNION u;								\
									\
    libc_feholdexcept_setround ## SUFFIX (&env, FE_TOWARDZERO);		\
    u.d = (EXPR);							\
    math_force_eval (u.d);						\
    u.ieee.MANTISSA							\
      |= libc_feupdateenv_test ## SUFFIX (&env, FE_INEXACT) != 0;	\
									\
    u.d;								\
  })

/* The following macros declare aliases for a narrowing function.  The
   sole argument is the base name of a family of functions, such as
   "add".  If any platform changes long double format after the
   introduction of narrowing functions, in a way requiring symbol
   versioning compatibility, additional variants of these macros will
   be needed.  */

#define libm_alias_float_double_main(func)	\
  weak_alias (__f ## func, f ## func)		\
  weak_alias (__f ## func, f32 ## func ## f64)	\
  weak_alias (__f ## func, f32 ## func ## f32x)

#ifdef NO_LONG_DOUBLE
# define libm_alias_float_double(func)		\
  libm_alias_float_double_main (func)		\
  weak_alias (__f ## func, f ## func ## l)
#else
# define libm_alias_float_double(func)		\
  libm_alias_float_double_main (func)
#endif

#define libm_alias_float32x_float64_main(func)			\
  weak_alias (__f32x ## func ## f64, f32x ## func ## f64)

#ifdef NO_LONG_DOUBLE
# define libm_alias_float32x_float64(func)		\
  libm_alias_float32x_float64_main (func)		\
  weak_alias (__f32x ## func ## f64, d ## func ## l)
#elif defined __LONG_DOUBLE_MATH_OPTIONAL
# define libm_alias_float32x_float64(func)			\
  libm_alias_float32x_float64_main (func)			\
  weak_alias (__f32x ## func ## f64, __nldbl_d ## func ## l)
#else
# define libm_alias_float32x_float64(func)	\
  libm_alias_float32x_float64_main (func)
#endif

#if __HAVE_FLOAT128 && !__HAVE_DISTINCT_FLOAT128
# define libm_alias_float_ldouble_f128(func)		\
  weak_alias (__f ## func ## l, f32 ## func ## f128)
# define libm_alias_double_ldouble_f128(func)		\
  weak_alias (__d ## func ## l, f32x ## func ## f128)	\
  weak_alias (__d ## func ## l, f64 ## func ## f128)
#else
# define libm_alias_float_ldouble_f128(func)
# define libm_alias_double_ldouble_f128(func)
#endif

#if __HAVE_FLOAT64X_LONG_DOUBLE
# define libm_alias_float_ldouble_f64x(func)		\
  weak_alias (__f ## func ## l, f32 ## func ## f64x)
# define libm_alias_double_ldouble_f64x(func)		\
  weak_alias (__d ## func ## l, f32x ## func ## f64x)	\
  weak_alias (__d ## func ## l, f64 ## func ## f64x)
#else
# define libm_alias_float_ldouble_f64x(func)
# define libm_alias_double_ldouble_f64x(func)
#endif

#define libm_alias_float_ldouble(func)		\
  weak_alias (__f ## func ## l, f ## func ## l) \
  libm_alias_float_ldouble_f128 (func)		\
  libm_alias_float_ldouble_f64x (func)

#define libm_alias_double_ldouble(func)		\
  weak_alias (__d ## func ## l, d ## func ## l) \
  libm_alias_double_ldouble_f128 (func)		\
  libm_alias_double_ldouble_f64x (func)

#define libm_alias_float64x_float128(func)			\
  weak_alias (__f64x ## func ## f128, f64x ## func ## f128)

#define libm_alias_float32_float128_main(func)			\
  weak_alias (__f32 ## func ## f128, f32 ## func ## f128)

#define libm_alias_float64_float128_main(func)			\
  weak_alias (__f64 ## func ## f128, f64 ## func ## f128)	\
  weak_alias (__f64 ## func ## f128, f32x ## func ## f128)

#if __HAVE_FLOAT64X_LONG_DOUBLE
# define libm_alias_float32_float128(func)	\
  libm_alias_float32_float128_main (func)
# define libm_alias_float64_float128(func)	\
  libm_alias_float64_float128_main (func)
#else
# define libm_alias_float32_float128(func)			\
  libm_alias_float32_float128_main (func)			\
  weak_alias (__f32 ## func ## f128, f32 ## func ## f64x)
# define libm_alias_float64_float128(func)			\
  libm_alias_float64_float128_main (func)			\
  weak_alias (__f64 ## func ## f128, f64 ## func ## f64x)	\
  weak_alias (__f64 ## func ## f128, f32x ## func ## f64x)
#endif

#endif /* math-narrow.h.  */
