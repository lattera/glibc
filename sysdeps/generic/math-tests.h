/* Configuration for math tests.  Generic version.
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

#include <bits/floatn.h>

/* Expand the appropriate macro for whether to enable tests for a
   given type.  */
#if __HAVE_DISTINCT_FLOAT128
# define MATH_TESTS_TG(PREFIX, ARGS, TYPE)				\
  (sizeof (TYPE) == sizeof (float) ? PREFIX ## float ARGS		\
   : sizeof (TYPE) == sizeof (double) ? PREFIX ## double ARGS		\
   : __builtin_types_compatible_p (TYPE, _Float128) ? PREFIX ## float128 ARGS \
   : PREFIX ## long_double ARGS)
# else
# define MATH_TESTS_TG(PREFIX, ARGS, TYPE)				\
  (sizeof (TYPE) == sizeof (float) ? PREFIX ## float ARGS		\
   : sizeof (TYPE) == sizeof (double) ? PREFIX ## double ARGS		\
   : PREFIX ## long_double ARGS)
#endif

/* Indicate whether to run tests involving sNaN values for the float, double,
   and long double C data types, respectively.  All are run unless
   overridden.  */
#ifndef SNAN_TESTS_float
# define SNAN_TESTS_float	1
#endif
#ifndef SNAN_TESTS_double
# define SNAN_TESTS_double	1
#endif
#ifndef SNAN_TESTS_long_double
# define SNAN_TESTS_long_double	1
#endif
#ifndef SNAN_TESTS_float128
# define SNAN_TESTS_float128	1
#endif

/* Return nonzero value if to run tests involving sNaN values for X.  */
#define SNAN_TESTS(x) MATH_TESTS_TG (SNAN_TESTS_, , x)

/* Indicate whether to run tests involving type casts of sNaN values.  These
   are run unless overridden.  */
#ifndef SNAN_TESTS_TYPE_CAST
# define SNAN_TESTS_TYPE_CAST	1
#endif

/* Indicate whether operations on signaling NaNs preserve the payload
   (if possible; it is not possible with a zero payload if the high
   bit is set for signaling NaNs) when generating a quiet NaN, and
   this should be tested.  */
#ifndef SNAN_TESTS_PRESERVE_PAYLOAD
# define SNAN_TESTS_PRESERVE_PAYLOAD	1
#endif

/* Indicate whether to run tests involving a given rounding mode for a
   given floating-point type, given that fesetround succeeds for that
   mode.  All are run if fesetround succeeds unless overridden.  */
#ifndef ROUNDING_TESTS_float
# define ROUNDING_TESTS_float(MODE)	1
#endif
#ifndef ROUNDING_TESTS_double
# define ROUNDING_TESTS_double(MODE)	1
#endif
#ifndef ROUNDING_TESTS_long_double
# define ROUNDING_TESTS_long_double(MODE)	1
#endif
#ifndef ROUNDING_TESTS_float128
# define ROUNDING_TESTS_float128(MODE)	1
#endif

#define ROUNDING_TESTS(TYPE, MODE)		\
  MATH_TESTS_TG (ROUNDING_TESTS_, (MODE), TYPE)

/* Indicate whether to run tests of floating-point exceptions for a
   given floating-point type, given that the exception macros are
   defined.  All are run unless overridden.  */
#ifndef EXCEPTION_TESTS_float
# define EXCEPTION_TESTS_float	1
#endif
#ifndef EXCEPTION_TESTS_double
# define EXCEPTION_TESTS_double	1
#endif
#ifndef EXCEPTION_TESTS_long_double
# define EXCEPTION_TESTS_long_double	1
#endif
#ifndef EXCEPTION_TESTS_float128
# define EXCEPTION_TESTS_float128	1
#endif

#define EXCEPTION_TESTS(TYPE) MATH_TESTS_TG (EXCEPTION_TESTS_, , TYPE)

/* Indicate whether the given exception trap(s) can be enabled
   in feenableexcept.  If non-zero, the traps are always supported.
   If zero, traps may or may not be supported depending on the
   target (this can be determined by checking the return value
   of feenableexcept).  This enables skipping of tests which use
   traps.  By default traps are supported unless overridden.  */
#ifndef EXCEPTION_ENABLE_SUPPORTED
# define EXCEPTION_ENABLE_SUPPORTED(EXCEPT)			\
   (EXCEPTION_TESTS_float || EXCEPTION_TESTS_double)
#endif

/* Indicate whether exception traps, if enabled, occur whenever an
   exception flag is set explicitly, so it is not possible to set flag
   bits with traps enabled without causing traps to be taken.  If
   traps cannot be enabled, the value of this macro does not
   matter.  */
#ifndef EXCEPTION_SET_FORCES_TRAP
# define EXCEPTION_SET_FORCES_TRAP 0
#endif
