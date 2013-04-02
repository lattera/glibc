/* Configuration for math tests.  Generic version.
   Copyright (C) 2013 Free Software Foundation, Inc.
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

/* Return nonzero value if to run tests involving sNaN values for X.  */
#define SNAN_TESTS(x)							\
  (sizeof (x) == sizeof (float) ? SNAN_TESTS_float			\
   : sizeof (x) == sizeof (double) ? SNAN_TESTS_double			\
   : SNAN_TESTS_long_double)

/* Indicate whether to run tests involving type casts of sNaN values.  These
   are run unless overridden.  */
#ifndef SNAN_TESTS_TYPE_CAST
# define SNAN_TESTS_TYPE_CAST	1
#endif
