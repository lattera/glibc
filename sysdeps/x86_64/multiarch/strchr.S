/* Multiple versions of strchr
   Copyright (C) 2009-2012 Free Software Foundation, Inc.
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


/* Define multiple versions only for the definition in libc.  */
#ifndef NOT_IN_libc
	.text
ENTRY(strchr)
	.type	strchr, @gnu_indirect_function
	cmpl	$0, __cpu_features+KIND_OFFSET(%rip)
	jne	1f
	call	__init_cpu_features
1:	leaq	__strchr_sse2(%rip), %rax
	testl	$bit_SSE4_2, __cpu_features+CPUID_OFFSET+index_SSE4_2(%rip)
	jz	2f
	leaq	__strchr_sse42(%rip), %rax
	ret
2:	testl	$bit_Slow_BSF, __cpu_features+FEATURE_OFFSET+index_Slow_BSF(%rip)
	jz	3f
	leaq    __strchr_sse2_no_bsf(%rip), %rax
3:	ret
END(strchr)


/*
   This implementation uses SSE4 instructions to compare up to 16 bytes
   at a time looking for the first occurrence of the character c in the
   string s:

   char *strchr (const char *s, int c);

   We use 0xa:
	_SIDD_SBYTE_OPS
	| _SIDD_CMP_EQUAL_EACH
	| _SIDD_LEAST_SIGNIFICANT
   on pcmpistri to compare xmm/mem128

   0 1 2 3 4 5 6 7 8 9 A B C D E F
   X X X X X X X X X X X X X X X X

   against xmm

   0 1 2 3 4 5 6 7 8 9 A B C D E F
   C C C C C C C C C C C C C C C C

   to find out if the first 16byte data element has a byte C and the
   offset of the first byte.  There are 3 cases:

   1. The first 16byte data element has the byte C at the offset X.
   2. The first 16byte data element has EOS and doesn't have the byte C.
   3. The first 16byte data element is valid and doesn't have the byte C.

   Here is the table of ECX, CFlag, ZFlag and SFlag for 3 cases:

   case		ECX	CFlag	ZFlag	SFlag
    1		 X	  1	 0/1	  0
    2		16	  0	  1	  0
    3		16	  0	  0	  0

   We exit from the loop for cases 1 and 2 with jbe which branches
   when either CFlag or ZFlag is 1.  If CFlag == 1, ECX has the offset
   X for case 1.  */

	.section .text.sse4.2,"ax",@progbits
	.align	16
	.type	__strchr_sse42, @function
	.globl	__strchr_sse42
	.hidden	__strchr_sse42
__strchr_sse42:
	cfi_startproc
	CALL_MCOUNT
	testb	%sil, %sil
	je	__strend_sse4
	pxor	%xmm2, %xmm2
	movd	%esi, %xmm1
	movl	%edi, %ecx
	pshufb  %xmm2, %xmm1
	andl	$15, %ecx
	movq	%rdi, %r8
	je	L(aligned_start)

/* Handle unaligned string.  */
	andq	$-16, %r8
	movdqa	(%r8), %xmm0
	pcmpeqb	 %xmm0, %xmm2
	pcmpeqb	 %xmm1, %xmm0
	/* Find where NULL is.  */
	pmovmskb %xmm2, %edx
	/* Check if there is a match.  */
	pmovmskb %xmm0, %esi
	/* Remove the leading  bytes.  */
	sarl	%cl, %edx
	sarl	%cl, %esi
	testl	%esi, %esi
	je	L(unaligned_no_match)
	/* Check which byte is a match.  */
	bsfl	%esi, %eax
	/* Is there a NULL? */
	testl	%edx, %edx
	je      L(unaligned_match)
	bsfl	%edx, %esi
	cmpl	%esi, %eax
	/* Return NULL if NULL comes first.  */
	ja	L(return_null)
L(unaligned_match):
	addq	%rdi, %rax
	ret

	.p2align 4
L(unaligned_no_match):
	testl	%edx, %edx
	jne	L(return_null)

/* Loop start on aligned string.  */
L(loop):
	addq	$16, %r8
L(aligned_start):
	pcmpistri	$0x2, (%r8), %xmm1
	jbe	L(wrap)
	addq	$16, %r8
	pcmpistri	$0x2, (%r8), %xmm1
	jbe	L(wrap)
	addq	$16, %r8
	pcmpistri       $0x2, (%r8), %xmm1
	jbe     L(wrap)
	addq	$16, %r8
	pcmpistri	$0x2, (%r8), %xmm1
	jbe	L(wrap)
	jmp	L(loop)
L(wrap):
	jc	L(loop_exit)

/* Return NULL.  */
L(return_null):
	xorl	%eax, %eax
	ret

/* Loop exit.  */
	.p2align 4
L(loop_exit):
	leaq	(%r8,%rcx), %rax
	ret
	cfi_endproc
	.size	__strchr_sse42, .-__strchr_sse42


# undef ENTRY
# define ENTRY(name) \
	.type __strchr_sse2, @function; \
	.align 16; \
	.globl __strchr_sse2; \
	.hidden __strchr_sse2; \
	__strchr_sse2: cfi_startproc; \
	CALL_MCOUNT
# undef END
# define END(name) \
	cfi_endproc; .size __strchr_sse2, .-__strchr_sse2
# undef libc_hidden_builtin_def
/* It doesn't make sense to send libc-internal strchr calls through a PLT.
   The speedup we get from using SSE4.2 instruction is likely eaten away
   by the indirect call in the PLT.  */
# define libc_hidden_builtin_def(name) \
	.globl __GI_strchr; __GI_strchr = __strchr_sse2
#endif

#include "../strchr.S"
