/* Configuration for math tests.  RISC-V version
   Copyright (C) 2014-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Copied from the aarch64 version

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

/* Trapping exceptions are not supported on RISC-V.  */
#define EXCEPTION_ENABLE_SUPPORTED(EXCEPT)	((EXCEPT) == 0)

/* Despite not supporting trapping exceptions, we support setting
   floating-point exception flags on hard-float targets.  These are not
   supported on soft-float targets.  */
#ifndef __riscv_flen
# define EXCEPTION_TESTS_float 0
# define EXCEPTION_TESTS_double        0
# define EXCEPTION_TESTS_long_double   0
#endif

/* On soft-float targets we only support the "to nearest" rounding mode.  */
#ifndef __riscv_flen
# define ROUNDING_TESTS_float(MODE)		((MODE) == FE_TONEAREST)
# define ROUNDING_TESTS_double(MODE)		((MODE) == FE_TONEAREST)
# define ROUNDING_TESTS_long_double(MODE)	((MODE) == FE_TONEAREST)
#endif

/* RISC-V floating-point instructions do not preserve NaN payloads.  */
#define SNAN_TESTS_PRESERVE_PAYLOAD	0

#include_next <math-tests.h>
