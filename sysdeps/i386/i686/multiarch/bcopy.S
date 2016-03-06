/* Multiple versions of bcopy
   All versions must be listed in ifunc-impl-list.c.
   Copyright (C) 2010-2016 Free Software Foundation, Inc.
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

/* Define multiple versions only for the definition in lib.  */
#if IS_IN (libc)
	.text
ENTRY(bcopy)
	.type	bcopy, @gnu_indirect_function
	LOAD_GOT_AND_RTLD_GLOBAL_RO
	LOAD_FUNC_GOT_EAX (__bcopy_ia32)
	HAS_CPU_FEATURE (SSE2)
	jz	2f
	LOAD_FUNC_GOT_EAX (__bcopy_sse2_unaligned)
	HAS_ARCH_FEATURE (Fast_Unaligned_Load)
	jnz	2f
	HAS_CPU_FEATURE (SSSE3)
	jz	2f
	LOAD_FUNC_GOT_EAX (__bcopy_ssse3)
	HAS_ARCH_FEATURE (Fast_Rep_String)
	jz	2f
	LOAD_FUNC_GOT_EAX (__bcopy_ssse3_rep)
2:	ret
END(bcopy)

# undef ENTRY
# define ENTRY(name) \
	.type __bcopy_ia32, @function; \
	.p2align 4; \
	.globl __bcopy_ia32; \
	.hidden __bcopy_ia32; \
	__bcopy_ia32: cfi_startproc; \
	CALL_MCOUNT
# undef END
# define END(name) \
	cfi_endproc; .size __bcopy_ia32, .-__bcopy_ia32

#endif

#include "../bcopy.S"
