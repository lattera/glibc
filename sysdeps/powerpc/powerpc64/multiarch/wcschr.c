/* Multiple versions of wcschr
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

#if IS_IN (libc)
# define wcschr __redirect_wcschr
# define __wcschr __redirect___wcschr
# include <wchar.h>
# include <shlib-compat.h>
# include "init-arch.h"

extern __typeof (wcschr) __wcschr_ppc attribute_hidden;
extern __typeof (wcschr) __wcschr_power6 attribute_hidden;
extern __typeof (wcschr) __wcschr_power7 attribute_hidden;
# undef wcschr
# undef __wcschr

libc_ifunc_redirected (__redirect___wcschr, __wcschr,
		       (hwcap & PPC_FEATURE_HAS_VSX)
		       ? __wcschr_power7
		       : (hwcap & PPC_FEATURE_ARCH_2_05)
			 ? __wcschr_power6
			 : __wcschr_ppc);
weak_alias (__wcschr, wcschr)
#else
#undef libc_hidden_def
#define libc_hidden_def(a)
#include <wcsmbs/wcschr.c>
#endif
