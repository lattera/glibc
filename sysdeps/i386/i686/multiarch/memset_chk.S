/* Multiple versions of __memset_chk
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
ENTRY(__memset_chk)
	.type	__memset_chk, @gnu_indirect_function
	LOAD_GOT_AND_RTLD_GLOBAL_RO
	LOAD_FUNC_GOT_EAX (__memset_chk_ia32)
	HAS_CPU_FEATURE (SSE2)
	jz	2f
	LOAD_FUNC_GOT_EAX (__memset_chk_sse2)
	HAS_ARCH_FEATURE (Fast_Rep_String)
	jz	2f
	LOAD_FUNC_GOT_EAX (__memset_chk_sse2_rep)
2:	ret
END(__memset_chk)

# ifdef SHARED
strong_alias (__memset_chk, __memset_zero_constant_len_parameter)
	.section .gnu.warning.__memset_zero_constant_len_parameter
	.string "memset used with constant zero length parameter; this could be due to transposed parameters"
# else
	.text
	.type __memset_chk_sse2, @function
	.p2align 4;
__memset_chk_sse2:
	cfi_startproc
	CALL_MCOUNT
	movl	12(%esp), %eax
	cmpl	%eax, 16(%esp)
	jb	__chk_fail
	jmp	__memset_sse2
	cfi_endproc
	.size __memset_chk_sse2, .-__memset_chk_sse2

	.type __memset_chk_sse2_rep, @function
	.p2align 4;
__memset_chk_sse2_rep:
	cfi_startproc
	CALL_MCOUNT
	movl	12(%esp), %eax
	cmpl	%eax, 16(%esp)
	jb	__chk_fail
	jmp	__memset_sse2_rep
	cfi_endproc
	.size __memset_chk_sse2_rep, .-__memset_chk_sse2_rep

	.type __memset_chk_ia32, @function
	.p2align 4;
__memset_chk_ia32:
	cfi_startproc
	CALL_MCOUNT
	movl	12(%esp), %eax
	cmpl	%eax, 16(%esp)
	jb	__chk_fail
	jmp	__memset_ia32
	cfi_endproc
	.size __memset_chk_ia32, .-__memset_chk_ia32
# endif
#endif
