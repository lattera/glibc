/* Configuration for math tests.  ARM version.
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

/* On systems with VFP support, but where glibc is built for
   soft-float, the libgcc functions used in libc and libm do not
   support rounding modes, although fesetround succeeds, and do not
   support exceptions.  */
#ifdef __SOFTFP__
# define ROUNDING_TESTS_float(MODE)	((MODE) == FE_TONEAREST)
# define ROUNDING_TESTS_double(MODE)	((MODE) == FE_TONEAREST)
# define ROUNDING_TESTS_long_double(MODE)	((MODE) == FE_TONEAREST)
# define EXCEPTION_TESTS_float	0
# define EXCEPTION_TESTS_double	0
# define EXCEPTION_TESTS_long_double	0
#endif

/* Not all VFP implementations support trapping exceptions.  */
#define EXCEPTION_ENABLE_SUPPORTED(EXCEPT)	((EXCEPT) == 0)

#include_next <math-tests.h>
