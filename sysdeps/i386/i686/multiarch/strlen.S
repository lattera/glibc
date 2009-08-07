/* Multiple versions of strlen
   Copyright (C) 2009 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <sysdep.h>
#include <ifunc-defines.h>

/* Define multiple versions only for the definition in libc and for the
   DSO.  In static binaries, we need strlen before the initialization
   happened.  */
#if defined SHARED && !defined NOT_IN_libc
	.section	.gnu.linkonce.t.__i686.get_pc_thunk.bx,"ax",@progbits
	.globl	__i686.get_pc_thunk.bx
	.hidden	__i686.get_pc_thunk.bx
	.p2align 4
	.type	__i686.get_pc_thunk.bx,@function
__i686.get_pc_thunk.bx:
	movl	(%esp), %ebx
	ret

	.text
ENTRY(strlen)
	.type	strlen, @gnu_indirect_function
	pushl	%ebx
	cfi_adjust_cfa_offset (4)
	cfi_rel_offset (ebx, 0)
	call	__i686.get_pc_thunk.bx
	addl	$_GLOBAL_OFFSET_TABLE_, %ebx
	cmpl	$0, KIND_OFFSET+__cpu_features@GOTOFF(%ebx)
	jne	1f
	call	__init_cpu_features
1:	leal	__strlen_ia32@GOTOFF(%ebx), %eax
	testl	$(1<<26), CPUID_OFFSET+COMMON_CPUID_INDEX_1*CPUID_SIZE+CPUID_EDX_OFFSET+__cpu_features@GOTOFF(%ebx)
	jz	2f
	leal	__strlen_sse2@GOTOFF(%ebx), %eax
2:	popl	%ebx
	cfi_adjust_cfa_offset (-4);
	cfi_restore (ebx)
	ret
END(strlen)

#define CFI_POP(REG)						\
  cfi_adjust_cfa_offset (-4);					\
  cfi_restore (REG)

#define RETURN		popl %esi; CFI_POP (esi); ret

	.text
ENTRY (__strlen_sse2)
/*
 * This implementation uses SSE instructions to compare up to 16 bytes
 * at a time looking for the end of string (null char).
 */
	pushl	%esi
	cfi_adjust_cfa_offset (4)
	cfi_rel_offset (%esi, 0)
	mov	8(%esp), %eax
	mov	%eax, %ecx
	pxor	%xmm0, %xmm0		/* 16 null chars */
	mov	%eax, %esi
	and	$15, %ecx
	jz	1f			/* string is 16 byte aligned */

	/*
	* Unaligned case. Round down to 16-byte boundary before comparing
	* 16 bytes for a null char. The code then compensates for any extra chars
	* preceding the start of the string.
	*/
	and	$-16, %esi

	pcmpeqb	(%esi), %xmm0
	lea	16(%eax), %esi
	pmovmskb %xmm0, %edx

	shr	%cl, %edx		/* Compensate for bytes preceding the string */
	test	%edx, %edx
	jnz	2f
	sub	%ecx, %esi		/* no null, adjust to next 16-byte boundary */
	pxor	%xmm0, %xmm0		/* clear xmm0, may have been changed... */

	.p2align 4
1:					/* 16 byte aligned */
	pcmpeqb	(%esi), %xmm0		/* look for null bytes */
	pmovmskb %xmm0, %edx		/* move each byte mask of %xmm0 to edx */

	add	$16, %esi		/* prepare to search next 16 bytes */
	test	%edx, %edx		/* if no null byte, %edx must be 0 */
	jnz	2f			/* found a null */

	pcmpeqb	(%esi), %xmm0
	pmovmskb %xmm0, %edx
	add	$16, %esi
	test	%edx, %edx
	jnz	2f

	pcmpeqb	(%esi), %xmm0
	pmovmskb %xmm0, %edx
	add	$16, %esi
	test	%edx, %edx
	jnz	2f

	pcmpeqb	(%esi), %xmm0
	pmovmskb %xmm0, %edx
	add	$16, %esi
	test	%edx, %edx
	jz	1b

2:
	neg	%eax
	lea	-16(%eax, %esi), %eax	/* calculate exact offset */
	bsf	%edx, %ecx		/* Least significant 1 bit is index of null */
	add	%ecx, %eax
	popl	%esi
	cfi_adjust_cfa_offset (-4)
	cfi_restore (%esi)
	ret

END (__strlen_sse2)

# undef ENTRY
# define ENTRY(name) \
	.type __strlen_ia32, @function; \
	.globl __strlen_ia32; \
	.p2align 4; \
	__strlen_ia32: cfi_startproc; \
	CALL_MCOUNT
# undef END
# define END(name) \
	cfi_endproc; .size __strlen_ia32, .-__strlen_ia32
# undef libc_hidden_builtin_def
/* IFUNC doesn't work with the hidden functions in shared library since
   they will be called without setting up EBX needed for PLT which is
   used by IFUNC.  */
# define libc_hidden_builtin_def(name) \
	.globl __GI_strlen; __GI_strlen = __strlen_ia32
#endif

#include "../../i586/strlen.S"
