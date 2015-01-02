/* Constant floating-point environments for e500.
   Copyright (C) 2004-2015 Free Software Foundation, Inc.
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

/* The use of "unsigned long long" as the type to define the
   bit-pattern explicitly, rather than the type "double" used in
   <bits/fenv.h>, means that we cannot include <fenv_libc.h> here to
   get the enum constants for the SPEFSCR bits to enable
   exceptions.  */

#include <sys/prctl.h>

/* If the default argument is used we use this value.  */
const unsigned long long __fe_dfl_env __attribute__ ((aligned (8))) =
  0x3cULL;

/* Floating-point environment where none of the exceptions are masked.  */
const unsigned long long __fe_enabled_env __attribute__ ((aligned (8))) =
  (((unsigned long long) (PR_FP_EXC_DIV
			  | PR_FP_EXC_OVF
			  | PR_FP_EXC_UND
			  | PR_FP_EXC_RES
			  | PR_FP_EXC_INV)) << 32) | 0x7cULL;

/* Non-IEEE mode.  */
const unsigned long long __fe_nonieee_env __attribute__ ((aligned (8))) =
  0x0ULL;
