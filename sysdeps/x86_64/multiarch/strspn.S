/* Multiple versions of strspn
   All versions must be listed in ifunc-impl-list.c.
   Copyright (C) 2009-2016 Free Software Foundation, Inc.
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

#include <config.h>
#include <sysdep.h>
#include <init-arch.h>

/* Define multiple versions only for the definition in libc.  */
#if IS_IN (libc)
	.text
ENTRY(strspn)
	.type	strspn, @gnu_indirect_function
	LOAD_RTLD_GLOBAL_RO_RDX
	leaq	__strspn_sse2(%rip), %rax
	HAS_CPU_FEATURE (SSE4_2)
	jz	2f
	leaq	__strspn_sse42(%rip), %rax
2:	ret
END(strspn)

# undef ENTRY
# define ENTRY(name) \
	.type __strspn_sse2, @function; \
	.globl __strspn_sse2; \
	.align 16; \
	__strspn_sse2: cfi_startproc; \
	CALL_MCOUNT
# undef END
# define END(name) \
	cfi_endproc; .size __strspn_sse2, .-__strspn_sse2
#endif

#include "../strspn.S"
