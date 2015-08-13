/* Multiple versions of __memmove_chk
   All versions must be listed in ifunc-impl-list.c.
   Copyright (C) 2010-2015 Free Software Foundation, Inc.
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
ENTRY(__memmove_chk)
	.type	__memmove_chk, @gnu_indirect_function
	LOAD_GOT_AND_RTLD_GLOBAL_RO
	LOAD_FUNC_GOT_EAX (__memmove_chk_ia32)
	HAS_CPU_FEATURE (SSE2)
	jz	2f
	LOAD_FUNC_GOT_EAX (__memmove_chk_sse2_unaligned)
	HAS_ARCH_FEATURE (Fast_Unaligned_Load)
	jnz	2f
	HAS_CPU_FEATURE (SSSE3)
	jz	2f
	LOAD_FUNC_GOT_EAX (__memmove_chk_ssse3)
	HAS_CPU_FEATURE (Fast_Rep_String)
	jz	2f
	LOAD_FUNC_GOT_EAX (__memmove_chk_ssse3_rep)
2:	ret
END(__memmove_chk)

# ifndef SHARED
	.type __memmove_chk_sse2_unaligned, @function
	.p2align 4;
__memmove_chk_sse2_unaligned:
	cfi_startproc
	CALL_MCOUNT
	movl	12(%esp), %eax
	cmpl	%eax, 16(%esp)
	jb	__chk_fail
	jmp	__memmove_sse2_unaligned
	cfi_endproc
	.size __memmove_chk_sse2_unaligned, .-__memmove_chk_sse2_unaligned

	.type __memmove_chk_ssse3, @function
	.p2align 4;
__memmove_chk_ssse3:
	cfi_startproc
	CALL_MCOUNT
	movl	12(%esp), %eax
	cmpl	%eax, 16(%esp)
	jb	__chk_fail
	jmp	__memmove_ssse3
	cfi_endproc
	.size __memmove_chk_ssse3, .-__memmove_chk_ssse3

	.type __memmove_chk_ssse3_rep, @function
	.p2align 4;
__memmove_chk_ssse3_rep:
	cfi_startproc
	CALL_MCOUNT
	movl	12(%esp), %eax
	cmpl	%eax, 16(%esp)
	jb	__chk_fail
	jmp	__memmove_ssse3_rep
	cfi_endproc
	.size __memmove_chk_ssse3_rep, .-__memmove_chk_ssse3_rep

	.type __memmove_chk_ia32, @function
	.p2align 4;
__memmove_chk_ia32:
	cfi_startproc
	CALL_MCOUNT
	movl	12(%esp), %eax
	cmpl	%eax, 16(%esp)
	jb	__chk_fail
	jmp	__memmove_ia32
	cfi_endproc
	.size __memmove_chk_ia32, .-__memmove_chk_ia32
# endif
#endif
