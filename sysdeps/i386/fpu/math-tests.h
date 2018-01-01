/* Configuration for math tests.  32-bit x86 version.
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

/* On 32-bit x86, versions of GCC up to at least 4.8 are happy to use FPU load
   instructions for sNaN values, and loading a float or double sNaN value will
   already raise an INVALID exception as well as turn the sNaN into a qNaN,
   rendering certain tests infeasible in this scenario.
   <http://gcc.gnu.org/PR56831>.  */
#define SNAN_TESTS_float	0
#define SNAN_TESTS_double	0

#include_next <math-tests.h>
