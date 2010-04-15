/* strcmp with SSE4.2
   Copyright (C) 2010 Free Software Foundation, Inc.
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

#ifndef NOT_IN_libc

#include <sysdep.h>
#include "asm-syntax.h"

#define CFI_PUSH(REG)						\
  cfi_adjust_cfa_offset (4);					\
  cfi_rel_offset (REG, 0)

#define CFI_POP(REG)						\
  cfi_adjust_cfa_offset (-4);					\
  cfi_restore (REG)

#define PUSH(REG)	pushl REG; CFI_PUSH (REG)
#define POP(REG)	popl REG; CFI_POP (REG)

#ifndef USE_AS_STRNCMP
# ifndef STRCMP
#  define STRCMP	__strcmp_sse4_2
# endif
# define STR1		4
# define STR2		STR1+4
# define RETURN		ret; .p2align 4
#else
# ifndef STRCMP
#  define STRCMP	__strncmp_sse4_2
# endif
# define STR1		8
# define STR2		STR1+4
# define CNT		STR2+4
# define RETURN		POP (%ebp); ret; .p2align 4; CFI_PUSH (%ebp)
#endif

	.section .text.sse4.2,"ax",@progbits
ENTRY (STRCMP)
#ifdef USE_AS_STRNCMP
	PUSH	(%ebp)
#endif
	mov	STR1(%esp), %edx
	mov	STR2(%esp), %eax
#ifdef USE_AS_STRNCMP
	movl	CNT(%esp), %ebp
	test	%ebp, %ebp
	je	L(eq)
#endif
	mov	%dx, %cx
	and	$0xfff, %cx
	cmp	$0xff0, %cx
	ja	L(first4bytes)
	movdqu	(%edx), %xmm2
	mov	%eax, %ecx
	and	$0xfff, %ecx
	cmp	$0xff0, %ecx
	ja	L(first4bytes)
	movd	%xmm2, %ecx
	cmp	(%eax), %ecx
	jne	L(less4bytes)
	movdqu	(%eax), %xmm1
	pxor	%xmm2, %xmm1
	pxor	%xmm0, %xmm0
	ptest	%xmm1, %xmm0
	jnc	L(less16bytes)
	pcmpeqb	%xmm0, %xmm2
	ptest	%xmm2, %xmm0
	jnc	L(less16bytes)

#ifdef USE_AS_STRNCMP
	sub	$16, %ebp
	jbe	L(eq)
#endif
	add	$16, %edx
	add	$16, %eax
L(first4bytes):
	movzbl	(%eax), %ecx
	cmpb	%cl, (%edx)
	jne	L(neq)
	cmpl	$0, %ecx
	je	L(eq)

#ifdef USE_AS_STRNCMP
	cmp	$1, %ebp
	je	L(eq)
#endif

	movzbl	1(%eax), %ecx
	cmpb	%cl, 1(%edx)
	jne	L(neq)
	cmpl	$0, %ecx
	je	L(eq)

#ifdef USE_AS_STRNCMP
	cmp	$2, %ebp
	je	L(eq)
#endif
	movzbl	2(%eax), %ecx
	cmpb	%cl, 2(%edx)
	jne	L(neq)
	cmpl	$0, %ecx
	je	L(eq)

#ifdef USE_AS_STRNCMP
	cmp	$3, %ebp
	je	L(eq)
#endif
	movzbl	3(%eax), %ecx
	cmpb	%cl, 3(%edx)
	jne	L(neq)
	cmpl	$0, %ecx
	je	L(eq)

#ifdef USE_AS_STRNCMP
	cmp	$4, %ebp
	je	L(eq)
#endif
	movzbl	4(%eax), %ecx
	cmpb	%cl, 4(%edx)
	jne	L(neq)
	cmpl	$0, %ecx
	je	L(eq)

#ifdef USE_AS_STRNCMP
	cmp	$5, %ebp
	je	L(eq)
#endif
	movzbl	5(%eax), %ecx
	cmpb	%cl, 5(%edx)
	jne	L(neq)
	cmpl	$0, %ecx
	je	L(eq)

#ifdef USE_AS_STRNCMP
	cmp	$6, %ebp
	je	L(eq)
#endif
	movzbl	6(%eax), %ecx
	cmpb	%cl, 6(%edx)
	jne	L(neq)
	cmpl	$0, %ecx
	je	L(eq)

#ifdef USE_AS_STRNCMP
	cmp	$7, %ebp
	je	L(eq)
#endif
	movzbl	7(%eax), %ecx
	cmpb	%cl, 7(%edx)
	jne	L(neq)
	cmpl	$0, %ecx
	je	L(eq)

#ifdef USE_AS_STRNCMP
	sub	$8, %ebp
	je	L(eq)
#endif
	add	$8, %eax
	add	$8, %edx

	PUSH	(%ebx)
	PUSH	(%edi)
	PUSH	(%esi)
#ifdef USE_AS_STRNCMP
	cfi_remember_state
#endif
	mov	%edx, %edi
	mov	%eax, %esi
	xorl	%eax, %eax
L(check_offset):
	movl	%edi, %ebx
	movl	%esi, %ecx
	andl	$0xfff, %ebx
	andl	$0xfff, %ecx
	cmpl	%ebx, %ecx
	cmovl	%ebx, %ecx
	lea	-0xff0(%ecx), %edx
	sub	%edx, %edi
	sub	%edx, %esi
	testl	%edx, %edx
	jg	L(crosspage)
L(loop):
	movdqu	(%esi,%edx), %xmm2
	movdqu	(%edi,%edx), %xmm1
	pcmpistri	$0x1a, %xmm2, %xmm1
	jbe	L(end)

#ifdef USE_AS_STRNCMP
	sub	$16, %ebp
	jbe	L(more16byteseq)
#endif

	add	$16, %edx
	jle	L(loop)
L(crosspage):
	movzbl	(%edi,%edx), %eax
	movzbl	(%esi,%edx), %ebx
	subl	%ebx, %eax
	jne	L(ret)
	testl	%ebx, %ebx
	je	L(ret)
#ifdef USE_AS_STRNCMP
	sub	$1, %ebp
	jbe	L(more16byteseq)
#endif
	inc	%edx
	cmp	$15, %edx
	jle	L(crosspage)
	add	%edx, %edi
	add	%edx, %esi
	jmp	L(check_offset)

	.p2align 4
L(end):
	jnc	L(ret)
#ifdef USE_AS_STRNCMP
	sub	%ecx, %ebp
	jbe	L(more16byteseq)
#endif
	lea	(%ecx,%edx), %ebx
	movzbl	(%edi,%ebx), %eax
	movzbl	(%esi,%ebx), %ecx
	subl	%ecx, %eax
L(ret):
	POP	(%esi)
	POP	(%edi)
	POP	(%ebx)
#ifdef USE_AS_STRNCMP
	POP	(%ebp)
#endif
	ret

	.p2align 4
#ifdef USE_AS_STRNCMP
	cfi_restore_state
L(more16byteseq):
	POP	(%esi)
	POP	(%edi)
	POP	(%ebx)
#endif
L(eq):
	xorl	%eax, %eax
	RETURN

L(neq):
	mov	$1, %eax
	ja	L(neq_bigger)
	neg	%eax
L(neq_bigger):
	RETURN

L(less16bytes):
	add	$0xfefefeff, %ecx
	jnc	L(less4bytes)
	xor	(%edx), %ecx
	or	$0xfefefeff, %ecx
	add	$1, %ecx
	jnz	L(less4bytes)

#ifdef USE_AS_STRNCMP
	cmp	$4, %ebp
	jbe	L(eq)
#endif
	mov	4(%edx), %ecx
	cmp	4(%eax), %ecx
	jne	L(more4bytes)
	add	$0xfefefeff, %ecx
	jnc	L(more4bytes)
	xor	4(%edx), %ecx
	or	$0xfefefeff, %ecx
	add	$1, %ecx
	jnz	L(more4bytes)

#ifdef USE_AS_STRNCMP
	sub	$8, %ebp
	jbe	L(eq)
#endif

	add	$8, %edx
	add	$8, %eax
L(less4bytes):

	movzbl	(%eax), %ecx
	cmpb	%cl, (%edx)
	jne	L(neq)
	cmpl	$0, %ecx
	je	L(eq)

#ifdef USE_AS_STRNCMP
	cmp	$1, %ebp
	je	L(eq)
#endif
	movzbl	1(%eax), %ecx
	cmpb	%cl, 1(%edx)
	jne	L(neq)
	cmpl	$0, %ecx
	je	L(eq)

#ifdef USE_AS_STRNCMP
	cmp	$2, %ebp
	je	L(eq)
#endif

	movzbl	2(%eax), %ecx
	cmpb	%cl, 2(%edx)
	jne	L(neq)
	cmpl	$0, %ecx
	je	L(eq)

#ifdef USE_AS_STRNCMP
	cmp	$3, %ebp
	je	L(eq)
#endif
	movzbl	3(%eax), %ecx
	cmpb	%cl, 3(%edx)
	jne	L(neq)
	cmpl	$0, %ecx
	je	L(eq)

L(more4bytes):
#ifdef USE_AS_STRNCMP
	cmp	$4, %ebp
	je	L(eq)
#endif
	movzbl	4(%eax), %ecx
	cmpb	%cl, 4(%edx)
	jne	L(neq)
	cmpl	$0, %ecx
	je	L(eq)


#ifdef USE_AS_STRNCMP
	cmp	$5, %ebp
	je	L(eq)
#endif
	movzbl	5(%eax), %ecx
	cmpb	%cl, 5(%edx)
	jne	L(neq)
	cmpl	$0, %ecx
	je	L(eq)

#ifdef USE_AS_STRNCMP
	cmp	$6, %ebp
	je	L(eq)
#endif
	movzbl	6(%eax), %ecx
	cmpb	%cl, 6(%edx)
	jne	L(neq)
	cmpl	$0, %ecx
	je	L(eq)

#ifdef USE_AS_STRNCMP
	cmp	$7, %ebp
	je	L(eq)
#endif
	movzbl	7(%eax), %ecx
	cmpb	%cl, 7(%edx)
	jne	L(neq)
	jmp	L(eq)

END (STRCMP)

#endif
