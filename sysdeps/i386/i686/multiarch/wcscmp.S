/* Multiple versions of wcscmp
   All versions must be listed in ifunc-impl-list.c.
   Copyright (C) 2011-2016 Free Software Foundation, Inc.
   Contributed by Intel Corporation.
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

#include <sysdep.h>
#include <init-arch.h>

/* Define multiple versions only for the definition in libc and for the
   DSO.  In static binaries, we need wcscmp before the initialization
   happened.  */
#if IS_IN (libc)
	.text
ENTRY(__wcscmp)
	.type	__wcscmp, @gnu_indirect_function
	LOAD_GOT_AND_RTLD_GLOBAL_RO
	LOAD_FUNC_GOT_EAX (__wcscmp_ia32)
	HAS_CPU_FEATURE (SSE2)
	jz	2f
	LOAD_FUNC_GOT_EAX (__wcscmp_sse2)
2:	ret
END(__wcscmp)
weak_alias (__wcscmp, wcscmp)
#endif
