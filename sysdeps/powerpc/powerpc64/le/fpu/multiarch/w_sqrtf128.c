/* Multiple versions of __sqrtf128.
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

#define NO_MATH_REDIRECT
#include <math.h>
#include "init-arch.h"
#include <math-type-macros-float128.h>

extern __typeof (__sqrtf128) __sqrtf128_ppc64le attribute_hidden;
extern __typeof (__sqrtf128) __sqrtf128_power9 attribute_hidden;

libc_ifunc (__sqrtf128,
	    (hwcap2 & PPC_FEATURE2_ARCH_3_00)
	    ? __sqrtf128_power9
	    : __sqrtf128_ppc64le);
declare_mgen_alias (__sqrt, sqrt)
