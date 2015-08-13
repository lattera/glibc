/* Multiple versions of wmemcmp
   All versions must be listed in ifunc-impl-list.c.
   Copyright (C) 2011-2015 Free Software Foundation, Inc.
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

/* Define multiple versions only for the definition in libc. */

#if IS_IN (libc)
	.text
ENTRY(wmemcmp)
	.type	wmemcmp, @gnu_indirect_function
	LOAD_GOT_AND_RTLD_GLOBAL_RO
	LOAD_FUNC_GOT_EAX (__wmemcmp_ia32)
	HAS_CPU_FEATURE (SSSE3)
	jz	2f
	LOAD_FUNC_GOT_EAX (__wmemcmp_ssse3)
	HAS_CPU_FEATURE (SSE4_2)
	jz	2f
	LOAD_FUNC_GOT_EAX (__wmemcmp_sse4_2)
2:	ret
END(wmemcmp)
#endif
