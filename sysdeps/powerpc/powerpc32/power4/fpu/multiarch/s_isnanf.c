/* Multiple versions of isnanf.
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

#include <math.h>
#include <shlib-compat.h>
#include "init-arch.h"

/* Both ppc32 and power7 isnan(double) work for float.  */
extern __typeof (__isnanf) __isnan_ppc32 attribute_hidden;
extern __typeof (__isnanf) __isnanf_power5 attribute_hidden;
extern __typeof (__isnanf) __isnanf_power6 attribute_hidden;
extern __typeof (__isnanf) __isnan_power7 attribute_hidden;

libc_ifunc_hidden (__isnanf, __isnanf,
		   (hwcap & PPC_FEATURE_ARCH_2_06)
		   ? __isnan_power7
		   : (hwcap & PPC_FEATURE_ARCH_2_05)
		     ? __isnanf_power6
		     : (hwcap & PPC_FEATURE_POWER5)
		       ? __isnanf_power5
		       : __isnan_ppc32);

hidden_def (__isnanf)
weak_alias (__isnanf, isnanf)
