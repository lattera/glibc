/* Entry point for multi-version x86 strncasecmp.
   All versions must be listed in ifunc-impl-list.c.
   Copyright (C) 2011-2016 Free Software Foundation, Inc.
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

	.text
ENTRY(__strncasecmp)
	.type	__strncasecmp, @gnu_indirect_function
	LOAD_GOT_AND_RTLD_GLOBAL_RO
	LOAD_FUNC_GOT_EAX (__strncasecmp_ia32)
	HAS_CPU_FEATURE (SSSE3)
	jz	2f
	LOAD_FUNC_GOT_EAX (__strncasecmp_ssse3)
	HAS_CPU_FEATURE (SSE4_2)
	jz	2f
	HAS_ARCH_FEATURE (Slow_SSE4_2)
	jnz	2f
	LOAD_FUNC_GOT_EAX (__strncasecmp_sse4_2)
2:	ret
END(__strncasecmp)

weak_alias (__strncasecmp, strncasecmp)
