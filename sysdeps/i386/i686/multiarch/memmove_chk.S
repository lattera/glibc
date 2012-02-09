/* Multiple versions of __memmove_chk
   Copyright (C) 2010, 2012 Free Software Foundation, Inc.
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
#ifndef NOT_IN_libc
# ifdef SHARED
	.text
ENTRY(__memmove_chk)
	.type	__memmove_chk, @gnu_indirect_function
	pushl	%ebx
	cfi_adjust_cfa_offset (4)
	cfi_rel_offset (ebx, 0)
	LOAD_PIC_REG(bx)
	cmpl	$0, KIND_OFFSET+__cpu_features@GOTOFF(%ebx)
	jne	1f
	call	__init_cpu_features
1:	leal	__memmove_chk_ia32@GOTOFF(%ebx), %eax
	testl	$bit_SSSE3, CPUID_OFFSET+index_SSSE3+__cpu_features@GOTOFF(%ebx)
	jz	2f
	leal	__memmove_chk_ssse3@GOTOFF(%ebx), %eax
	testl	$bit_Fast_Rep_String, FEATURE_OFFSET+index_Fast_Rep_String+__cpu_features@GOTOFF(%ebx)
	jz	2f
	leal	__memmove_chk_ssse3_rep@GOTOFF(%ebx), %eax
2:	popl	%ebx
	cfi_adjust_cfa_offset (-4)
	cfi_restore (ebx)
	ret
END(__memmove_chk)
# else
	.text
ENTRY(__memmove_chk)
	.type	__memmove_chk, @gnu_indirect_function
	cmpl	$0, KIND_OFFSET+__cpu_features
	jne	1f
	call	__init_cpu_features
1:	leal	__memmove_chk_ia32, %eax
	testl	$bit_SSSE3, CPUID_OFFSET+index_SSSE3+__cpu_features
	jz	2f
	leal	__memmove_chk_ssse3, %eax
	testl	$bit_Fast_Rep_String, FEATURE_OFFSET+index_Fast_Rep_String+__cpu_features
	jz	2f
	leal	__memmove_chk_ssse3_rep, %eax
2:	ret
END(__memmove_chk)

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
