/* Multiple versions of wcsrchr
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

#if IS_IN (libc)
# include <wchar.h>
# include <shlib-compat.h>
# include "init-arch.h"

extern __typeof (wcsrchr) __wcsrchr_ppc attribute_hidden;
extern __typeof (wcsrchr) __wcsrchr_power6 attribute_hidden;
extern __typeof (wcsrchr) __wcsrchr_power7 attribute_hidden;

libc_ifunc (wcsrchr,
	     (hwcap & PPC_FEATURE_HAS_VSX)
             ? __wcsrchr_power7 :
	       (hwcap & PPC_FEATURE_ARCH_2_05)
	       ? __wcsrchr_power6
             : __wcsrchr_ppc);
#else
#include <wcsmbs/wcsrchr.c>
#endif
