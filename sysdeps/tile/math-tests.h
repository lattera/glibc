/* Configuration for math tests.  Tile version.
   Copyright (C) 2013-2016 Free Software Foundation, Inc.
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

/* Tile hardware/softfloat does not support exceptions and rounding modes.  */
#define ROUNDING_TESTS_float(MODE)	((MODE) == FE_TONEAREST)
#define ROUNDING_TESTS_double(MODE)	((MODE) == FE_TONEAREST)
#define ROUNDING_TESTS_long_double(MODE)	((MODE) == FE_TONEAREST)
#define EXCEPTION_TESTS_float	0
#define EXCEPTION_TESTS_double	0
#define EXCEPTION_TESTS_long_double	0

/* Tile hardware/softfloat floating-point ops do not preserve NaN payloads.  */
#define SNAN_TESTS_PRESERVE_PAYLOAD	0

#include_next <math-tests.h>
