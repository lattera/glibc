/* Multiple versions of isnan.
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

#include <math.h>
#include "init-arch.h"

/* The double-precision implementation also works for the single one.  */
extern __typeof (__isnanf) __isnan_ppc64 attribute_hidden;
extern __typeof (__isnanf) __isnan_power5 attribute_hidden;
extern __typeof (__isnanf) __isnan_power6 attribute_hidden;
extern __typeof (__isnanf) __isnan_power6x attribute_hidden;
extern __typeof (__isnanf) __isnan_power7 attribute_hidden;
extern __typeof (__isnanf) __isnan_power8 attribute_hidden;

libc_ifunc_hidden (__isnanf, __isnanf,
		   (hwcap2 & PPC_FEATURE2_ARCH_2_07)
		   ? __isnan_power8
		   : (hwcap & PPC_FEATURE_ARCH_2_06)
		     ? __isnan_power7
		     : (hwcap & PPC_FEATURE_POWER6_EXT)
		       ? __isnan_power6x
		       : (hwcap & PPC_FEATURE_ARCH_2_05)
			 ? __isnan_power6
			 : (hwcap & PPC_FEATURE_POWER5)
			   ? __isnan_power5
			   : __isnan_ppc64);

hidden_def (__isnanf)
weak_alias (__isnanf, isnanf)
