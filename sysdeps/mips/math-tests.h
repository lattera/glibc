/* Configuration for math tests.  MIPS version.
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

#include <features.h>
#include <sgidefs.h>

/* MIPS soft float does not support exceptions and rounding modes, and
   before GCC 4.9 long double when wider than double is implemented
   using fp-bit which does not integrate with hardware exceptions and
   rounding modes.  */
#ifdef __mips_soft_float
# define ROUNDING_TESTS_float(MODE)	((MODE) == FE_TONEAREST)
# define ROUNDING_TESTS_double(MODE)	((MODE) == FE_TONEAREST)
# define ROUNDING_TESTS_long_double(MODE)	((MODE) == FE_TONEAREST)
# define EXCEPTION_TESTS_float	0
# define EXCEPTION_TESTS_double	0
# define EXCEPTION_TESTS_long_double	0
#elif _MIPS_SIM != _ABIO32 && !__GNUC_PREREQ (4, 9)
# define ROUNDING_TESTS_long_double(MODE)	((MODE) == FE_TONEAREST)
# define EXCEPTION_TESTS_long_double	0
#endif

/* NaN payload preservation when converting a signaling NaN to quiet
   is only required in NAN2008 mode.  */
#if defined __mips_hard_float && !defined __mips_nan2008
# define SNAN_TESTS_PRESERVE_PAYLOAD	0
#endif

#include_next <math-tests.h>
