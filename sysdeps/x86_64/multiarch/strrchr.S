/* strrchr with SSE4.2
   Copyright (C) 2009 Free Software Foundation, Inc.
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
#include <init-arch.h>


/* Define multiple versions only for the definition in libc and for
   the DSO.  In static binaries we need strrchr before the initialization
   happened.  */
#if defined SHARED && !defined NOT_IN_libc
	.text
ENTRY(strrchr)
	.type	strrchr, @gnu_indirect_function
	cmpl	$0, __cpu_features+KIND_OFFSET(%rip)
	jne	1f
	call	__init_cpu_features
1:	leaq	__strrchr_sse2(%rip), %rax
	testl	$bit_SSE4_2, __cpu_features+CPUID_OFFSET+index_SSE4_2(%rip)
	jz	2f
	leaq	__strrchr_sse42(%rip), %rax
	ret
2:	testl	$bit_Slow_BSF, __cpu_features+FEATURE_OFFSET+index_Slow_BSF(%rip)
	jz	3f
	leaq    __strrchr_sse2_no_bsf(%rip), %rax
3:	ret
END(strrchr)

/*
   This implementation uses SSE4 instructions to compare up to 16 bytes
   at a time looking for the last occurrence of the character c in the
   string s:

   char *strrchr (const char *s, int c);

   We use 0x4a:
	_SIDD_SBYTE_OPS
	| _SIDD_CMP_EQUAL_EACH
	| _SIDD_MOST_SIGNIFICANT
   on pcmpistri to compare xmm/mem128

   0 1 2 3 4 5 6 7 8 9 A B C D E F
   X X X X X X X X X X X X X X X X

   against xmm

   0 1 2 3 4 5 6 7 8 9 A B C D E F
   C C C C C C C C C C C C C C C C

   to find out if the first 16byte data element has a byte C and the
   last offset.  There are 4 cases:

   1. The first 16byte data element has EOS and has the byte C at the
      last offset X.
   2. The first 16byte data element is valid and has the byte C at the
      last offset X.
   3. The first 16byte data element has EOS and doesn't have the byte C.
   4. The first 16byte data element is valid and doesn't have the byte C.

   Here is the table of ECX, CFlag, ZFlag and SFlag for 3 cases:

   case		ECX	CFlag	ZFlag	SFlag
    1		 X	  1	  1	  0
    2		 X	  1	  0	  0
    3		16	  0	  1	  0
    4		16	  0	  0	  0

   We exit from the loop for cases 1 and 3 with jz which branches
   when ZFlag is 1.  If CFlag == 1, ECX has the offset X for case 1.  */


	.section .text.sse4.2,"ax",@progbits
	.align	16
	.type	__strrchr_sse42, @function
__strrchr_sse42:
	cfi_startproc
	CALL_MCOUNT
	testb	%sil, %sil
	je	__strend_sse4
	xor	%eax,%eax	/* RAX has the last occurrence of s.  */
	movd	%esi, %xmm1
	punpcklbw	%xmm1, %xmm1
	movl	%edi, %esi
	punpcklbw	%xmm1, %xmm1
	andl	$15, %esi
	pshufd	$0, %xmm1, %xmm1
	movq	%rdi, %r8
	je	L(loop)

/* Handle unaligned string using psrldq.  */
	leaq	L(psrldq_table)(%rip), %rdx
	andq	$-16, %r8
	movslq	(%rdx,%rsi,4),%r9
	movdqa	(%r8), %xmm0
	addq	%rdx, %r9
	jmp	*%r9

/* Handle unaligned string with offset 1 using psrldq.  */
	.p2align 4
L(psrldq_1):
	psrldq	$1, %xmm0

	.p2align 4
L(unaligned_pcmpistri):
	pcmpistri	$0x4a, %xmm1, %xmm0
	jnc	L(unaligned_no_byte)
	leaq	(%rdi,%rcx), %rax
L(unaligned_no_byte):
	/* Find the length of the unaligned string.  */
	pcmpistri	$0x3a, %xmm0, %xmm0
	movl	$16, %edx
	subl	%esi, %edx
	cmpl	%ecx, %edx
	/* Return RAX if the unaligned fragment to next 16B already
	   contain the NULL terminator.  */
	jg	L(exit)
	addq	$16, %r8

/* Loop start on aligned string.  */
	.p2align 4
L(loop):
	pcmpistri	$0x4a, (%r8), %xmm1
	jbe	L(match_or_eos)
	addq	$16, %r8
	jmp	L(loop)
	.p2align 4
L(match_or_eos):
	je	L(had_eos)
L(match_no_eos):
	leaq	(%r8,%rcx), %rax
	addq	$16, %r8
	jmp     L(loop)
	.p2align 4
L(had_eos):
	jnc     L(exit)
	leaq	(%r8,%rcx), %rax
	.p2align 4
L(exit):
	ret

/* Handle unaligned string with offset 15 using psrldq.  */
	.p2align 4
L(psrldq_15):
	psrldq	$15, %xmm0
	jmp	L(unaligned_pcmpistri)

/* Handle unaligned string with offset 14 using psrldq.  */
	.p2align 4
L(psrldq_14):
	psrldq	$14, %xmm0
	jmp	L(unaligned_pcmpistri)

/* Handle unaligned string with offset 13 using psrldq.  */
	.p2align 4
L(psrldq_13):
	psrldq	$13, %xmm0
	jmp	L(unaligned_pcmpistri)

/* Handle unaligned string with offset 12 using psrldq.  */
	.p2align 4
L(psrldq_12):
	psrldq	$12, %xmm0
	jmp	L(unaligned_pcmpistri)

/* Handle unaligned string with offset 11 using psrldq.  */
	.p2align 4
L(psrldq_11):
	psrldq	$11, %xmm0
	jmp	L(unaligned_pcmpistri)

/* Handle unaligned string with offset 10 using psrldq.  */
	.p2align 4
L(psrldq_10):
	psrldq	$10, %xmm0
	jmp	L(unaligned_pcmpistri)

/* Handle unaligned string with offset 9 using psrldq.  */
	.p2align 4
L(psrldq_9):
	psrldq	$9, %xmm0
	jmp	L(unaligned_pcmpistri)

/* Handle unaligned string with offset 8 using psrldq.  */
	.p2align 4
L(psrldq_8):
	psrldq	$8, %xmm0
	jmp	L(unaligned_pcmpistri)

/* Handle unaligned string with offset 7 using psrldq.  */
	.p2align 4
L(psrldq_7):
	psrldq	$7, %xmm0
	jmp	L(unaligned_pcmpistri)

/* Handle unaligned string with offset 6 using psrldq.  */
	.p2align 4
L(psrldq_6):
	psrldq	$6, %xmm0
	jmp	L(unaligned_pcmpistri)

/* Handle unaligned string with offset 5 using psrldq.  */
	.p2align 4
L(psrldq_5):
	psrldq	$5, %xmm0
	jmp	L(unaligned_pcmpistri)

/* Handle unaligned string with offset 4 using psrldq.  */
	.p2align 4
L(psrldq_4):
	psrldq	$4, %xmm0
	jmp	L(unaligned_pcmpistri)

/* Handle unaligned string with offset 3 using psrldq.  */
	.p2align 4
L(psrldq_3):
	psrldq	$3, %xmm0
	jmp	L(unaligned_pcmpistri)

/* Handle unaligned string with offset 2 using psrldq.  */
	.p2align 4
L(psrldq_2):
	psrldq	$2, %xmm0
	jmp	L(unaligned_pcmpistri)

	cfi_endproc
	.size	__strrchr_sse42, .-__strrchr_sse42

	.section .rodata.sse4.2,"a",@progbits
	.p2align 4
L(psrldq_table):
	.int	L(loop) - L(psrldq_table)
	.int	L(psrldq_1) - L(psrldq_table)
	.int	L(psrldq_2) - L(psrldq_table)
	.int	L(psrldq_3) - L(psrldq_table)
	.int	L(psrldq_4) - L(psrldq_table)
	.int	L(psrldq_5) - L(psrldq_table)
	.int	L(psrldq_6) - L(psrldq_table)
	.int	L(psrldq_7) - L(psrldq_table)
	.int	L(psrldq_8) - L(psrldq_table)
	.int	L(psrldq_9) - L(psrldq_table)
	.int	L(psrldq_10) - L(psrldq_table)
	.int	L(psrldq_11) - L(psrldq_table)
	.int	L(psrldq_12) - L(psrldq_table)
	.int	L(psrldq_13) - L(psrldq_table)
	.int	L(psrldq_14) - L(psrldq_table)
	.int	L(psrldq_15) - L(psrldq_table)


# undef ENTRY
# define ENTRY(name) \
	.type __strrchr_sse2, @function; \
	.align 16; \
	__strrchr_sse2: cfi_startproc; \
	CALL_MCOUNT
# undef END
# define END(name) \
	cfi_endproc; .size __strrchr_sse2, .-__strrchr_sse2
# undef libc_hidden_builtin_def
/* It doesn't make sense to send libc-internal strrchr calls through a PLT.
   The speedup we get from using SSE4.2 instruction is likely eaten away
   by the indirect call in the PLT.  */
# define libc_hidden_builtin_def(name) \
	.globl __GI_strrchr; __GI_strrchr = __strrchr_sse2
#endif

#include "../strrchr.S"
