/* Multiple versions of finitef.
   Copyright (C) 2013-2015 Free Software Foundation, Inc.
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

#include <math.h>
#include <shlib-compat.h>
#include "init-arch.h"

extern __typeof (__finitef) __finitef_ppc64 attribute_hidden;
/* The double-precision version also works for single-precision.  */
extern __typeof (__finitef) __finite_power7 attribute_hidden;
extern __typeof (__finitef) __finite_power8 attribute_hidden;

libc_ifunc (__finitef,
	    (hwcap2 & PPC_FEATURE2_ARCH_2_07)
	    ? __finite_power8 :
	      (hwcap & PPC_FEATURE_ARCH_2_06)
	      ? __finite_power7
            : __finitef_ppc64);

weak_alias (__finitef, finitef)
