/* strcat with SSSE3
   Copyright (C) 2011 Free Software Foundation, Inc.
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

# include <sysdep.h>

# ifndef STRCAT
#  define STRCAT  __strcat_ssse3
# endif

# define USE_AS_STRCAT

.text
ENTRY (STRCAT)
# ifdef USE_AS_STRNCAT
	mov	%rdx, %r8
# endif

# define RETURN  jmp L(StartStrcpyPart)
# include "strlen-no-bsf.S"

# undef RETURN

L(StartStrcpyPart):
	mov	%rsi, %rcx
	lea	(%rdi, %rax), %rdx
# ifdef USE_AS_STRNCAT
	test	%r8, %r8
	jz	L(StrncatExit0)
	cmp	$8, %r8
	jbe	L(StrncatExit8Bytes)
# endif
	cmpb	$0, (%rcx)
	jz	L(Exit1)
	cmpb	$0, 1(%rcx)
	jz	L(Exit2)
	cmpb	$0, 2(%rcx)
	jz	L(Exit3)
	cmpb	$0, 3(%rcx)
	jz	L(Exit4)
	cmpb	$0, 4(%rcx)
	jz	L(Exit5)
	cmpb	$0, 5(%rcx)
	jz	L(Exit6)
	cmpb	$0, 6(%rcx)
	jz	L(Exit7)
	cmpb	$0, 7(%rcx)
	jz	L(Exit8)
	cmpb	$0, 8(%rcx)
	jz	L(Exit9)
# ifdef USE_AS_STRNCAT
	cmp	$16, %r8
	jb	L(StrncatExit15Bytes)
# endif
	cmpb	$0, 9(%rcx)
	jz	L(Exit10)
	cmpb	$0, 10(%rcx)
	jz	L(Exit11)
	cmpb	$0, 11(%rcx)
	jz	L(Exit12)
	cmpb	$0, 12(%rcx)
	jz	L(Exit13)
	cmpb	$0, 13(%rcx)
	jz	L(Exit14)
	cmpb	$0, 14(%rcx)
	jz	L(Exit15)
	cmpb	$0, 15(%rcx)
	jz	L(Exit16)
# ifdef USE_AS_STRNCAT
	cmp	$16, %r8
	je	L(StrncatExit16)
#  define USE_AS_STRNCPY
# endif

# include "strcpy-ssse3.S"

	.p2align 4
L(CopyFrom1To16Bytes):
	add	%rsi, %rdx
	add	%rsi, %rcx

	test	%al, %al
	jz	L(ExitHigh)
	test	$0x01, %al
	jnz	L(Exit1)
	test	$0x02, %al
	jnz	L(Exit2)
	test	$0x04, %al
	jnz	L(Exit3)
	test	$0x08, %al
	jnz	L(Exit4)
	test	$0x10, %al
	jnz	L(Exit5)
	test	$0x20, %al
	jnz	L(Exit6)
	test	$0x40, %al
	jnz	L(Exit7)
	movlpd	(%rcx), %xmm0
	movlpd	%xmm0, (%rdx)
	mov	%rdi, %rax
	ret

	.p2align 4
L(ExitHigh):
	test	$0x01, %ah
	jnz	L(Exit9)
	test	$0x02, %ah
	jnz	L(Exit10)
	test	$0x04, %ah
	jnz	L(Exit11)
	test	$0x08, %ah
	jnz	L(Exit12)
	test	$0x10, %ah
	jnz	L(Exit13)
	test	$0x20, %ah
	jnz	L(Exit14)
	test	$0x40, %ah
	jnz	L(Exit15)
	movlpd	(%rcx), %xmm0
	movlpd	8(%rcx), %xmm1
	movlpd	%xmm0, (%rdx)
	movlpd	%xmm1, 8(%rdx)
	mov	%rdi, %rax
	ret

	.p2align 4
L(StrncatExit1):
	xor	%ah, %ah
	movb	%ah, 1(%rdx)
L(Exit1):
	movb	(%rcx), %al
	movb	%al, (%rdx)
	mov	%rdi, %rax
	ret

	.p2align 4
L(StrncatExit2):
	xor	%ah, %ah
	movb	%ah, 2(%rdx)
L(Exit2):
	movw	(%rcx), %ax
	movw	%ax, (%rdx)
	mov	%rdi, %rax
	ret

	.p2align 4
L(StrncatExit3):
	xor	%ah, %ah
	movb	%ah, 3(%rdx)
L(Exit3):
	movw	(%rcx), %ax
	movw	%ax, (%rdx)
	movb	2(%rcx), %al
	movb	%al, 2(%rdx)
	mov	%rdi, %rax
	ret

	.p2align 4
L(StrncatExit4):
	xor	%ah, %ah
	movb	%ah, 4(%rdx)
L(Exit4):
	mov	(%rcx), %eax
	mov	%eax, (%rdx)
	mov	%rdi, %rax
	ret

	.p2align 4
L(StrncatExit5):
	xor	%ah, %ah
	movb	%ah, 5(%rdx)
L(Exit5):
	mov	(%rcx), %eax
	mov	%eax, (%rdx)
	movb	4(%rcx), %al
	movb	%al, 4(%rdx)
	mov	%rdi, %rax
	ret

	.p2align 4
L(StrncatExit6):
	xor	%ah, %ah
	movb	%ah, 6(%rdx)
L(Exit6):
	mov	(%rcx), %eax
	mov	%eax, (%rdx)
	movw	4(%rcx), %ax
	movw	%ax, 4(%rdx)
	mov	%rdi, %rax
	ret

	.p2align 4
L(StrncatExit7):
	xor	%ah, %ah
	movb	%ah, 7(%rdx)
L(Exit7):
	mov	(%rcx), %eax
	mov	%eax, (%rdx)
	mov	3(%rcx), %eax
	mov	%eax, 3(%rdx)
	mov	%rdi, %rax
	ret

	.p2align 4
L(StrncatExit8):
	xor	%ah, %ah
	movb	%ah, 8(%rdx)
L(Exit8):
	movlpd	(%rcx), %xmm0
	movlpd	%xmm0, (%rdx)
	mov	%rdi, %rax
	ret

	.p2align 4
L(StrncatExit9):
	xor	%ah, %ah
	movb	%ah, 9(%rdx)
L(Exit9):
	movlpd	(%rcx), %xmm0
	movlpd	%xmm0, (%rdx)
	movb	8(%rcx), %al
	movb	%al, 8(%rdx)
	mov	%rdi, %rax
	ret

	.p2align 4
L(StrncatExit10):
	xor	%ah, %ah
	movb	%ah, 10(%rdx)
L(Exit10):
	movlpd	(%rcx), %xmm0
	movlpd	%xmm0, (%rdx)
	movw	8(%rcx), %ax
	movw	%ax, 8(%rdx)
	mov	%rdi, %rax
	ret

	.p2align 4
L(StrncatExit11):
	xor	%ah, %ah
	movb	%ah, 11(%rdx)
L(Exit11):
	movlpd	(%rcx), %xmm0
	movlpd	%xmm0, (%rdx)
	mov	7(%rcx), %eax
	mov	%eax, 7(%rdx)
	mov	%rdi, %rax
	ret

	.p2align 4
L(StrncatExit12):
	xor	%ah, %ah
	movb	%ah, 12(%rdx)
L(Exit12):
	movlpd	(%rcx), %xmm0
	movlpd	%xmm0, (%rdx)
	mov	8(%rcx), %eax
	mov	%eax, 8(%rdx)
	mov	%rdi, %rax
	ret

	.p2align 4
L(StrncatExit13):
	xor	%ah, %ah
	movb	%ah, 13(%rdx)
L(Exit13):
	movlpd	(%rcx), %xmm0
	movlpd	%xmm0, (%rdx)
	movlpd	5(%rcx), %xmm1
	movlpd	%xmm1, 5(%rdx)
	mov	%rdi, %rax
	ret

	.p2align 4
L(StrncatExit14):
	xor	%ah, %ah
	movb	%ah, 14(%rdx)
L(Exit14):
	movlpd	(%rcx), %xmm0
	movlpd	%xmm0, (%rdx)
	movlpd	6(%rcx), %xmm1
	movlpd	%xmm1, 6(%rdx)
	mov	%rdi, %rax
	ret

	.p2align 4
L(StrncatExit15):
	xor	%ah, %ah
	movb	%ah, 15(%rdx)
L(Exit15):
	movlpd	(%rcx), %xmm0
	movlpd	%xmm0, (%rdx)
	movlpd	7(%rcx), %xmm1
	movlpd	%xmm1, 7(%rdx)
	mov	%rdi, %rax
	ret

	.p2align 4
L(StrncatExit16):
	xor	%ah, %ah
	movb	%ah, 16(%rdx)
L(Exit16):
	movlpd	(%rcx), %xmm0
	movlpd	8(%rcx), %xmm1
	movlpd	%xmm0, (%rdx)
	movlpd	%xmm1, 8(%rdx)
	mov	%rdi, %rax
	ret

# ifdef USE_AS_STRNCPY

	.p2align 4
L(CopyFrom1To16BytesCase2):
	add	$16, %r8
	add	%rsi, %rcx
	lea	(%rsi, %rdx), %rsi
	lea	-9(%r8), %rdx
	and	$1<<7, %dh
	or	%al, %dh
	test	%dh, %dh
	lea	(%rsi), %rdx
	jz	L(ExitHighCase2)

	test	$0x01, %al
	jnz	L(Exit1)
	cmp	$1, %r8
	je	L(StrncatExit1)
	test	$0x02, %al
	jnz	L(Exit2)
	cmp	$2, %r8
	je	L(StrncatExit2)
	test	$0x04, %al
	jnz	L(Exit3)
	cmp	$3, %r8
	je	L(StrncatExit3)
	test	$0x08, %al
	jnz	L(Exit4)
	cmp	$4, %r8
	je	L(StrncatExit4)
	test	$0x10, %al
	jnz	L(Exit5)
	cmp	$5, %r8
	je	L(StrncatExit5)
	test	$0x20, %al
	jnz	L(Exit6)
	cmp	$6, %r8
	je	L(StrncatExit6)
	test	$0x40, %al
	jnz	L(Exit7)
	cmp	$7, %r8
	je	L(StrncatExit7)
	movlpd	(%rcx), %xmm0
	movlpd	%xmm0, (%rdx)
	lea	7(%rdx), %rax
	cmpb	$1, (%rax)
	sbb	$-1, %rax
	xor	%cl, %cl
	movb	%cl, (%rax)
	mov	%rdi, %rax
	ret

	.p2align 4
L(ExitHighCase2):
	test	$0x01, %ah
	jnz	L(Exit9)
	cmp	$9, %r8
	je	L(StrncatExit9)
	test	$0x02, %ah
	jnz	L(Exit10)
	cmp	$10, %r8
	je	L(StrncatExit10)
	test	$0x04, %ah
	jnz	L(Exit11)
	cmp	$11, %r8
	je	L(StrncatExit11)
	test	$0x8, %ah
	jnz	L(Exit12)
	cmp	$12, %r8
	je	L(StrncatExit12)
	test	$0x10, %ah
	jnz	L(Exit13)
	cmp	$13, %r8
	je	L(StrncatExit13)
	test	$0x20, %ah
	jnz	L(Exit14)
	cmp	$14, %r8
	je	L(StrncatExit14)
	test	$0x40, %ah
	jnz	L(Exit15)
	cmp	$15, %r8
	je	L(StrncatExit15)
	movlpd	(%rcx), %xmm0
	movlpd	%xmm0, (%rdx)
	movlpd	8(%rcx), %xmm1
	movlpd	%xmm1, 8(%rdx)
	mov	%rdi, %rax
	ret

L(CopyFrom1To16BytesCase2OrCase3):
	test	%rax, %rax
	jnz	L(CopyFrom1To16BytesCase2)

	.p2align 4
L(CopyFrom1To16BytesCase3):
	add	$16, %r8
	add	%rsi, %rdx
	add	%rsi, %rcx

	cmp	$8, %r8
	ja	L(ExitHighCase3)
	cmp	$1, %r8
	je	L(StrncatExit1)
	cmp	$2, %r8
	je	L(StrncatExit2)
	cmp	$3, %r8
	je	L(StrncatExit3)
	cmp	$4, %r8
	je	L(StrncatExit4)
	cmp	$5, %r8
	je	L(StrncatExit5)
	cmp	$6, %r8
	je	L(StrncatExit6)
	cmp	$7, %r8
	je	L(StrncatExit7)
	movlpd	(%rcx), %xmm0
	movlpd	%xmm0, (%rdx)
	xor	%ah, %ah
	movb	%ah, 8(%rdx)
	mov	%rdi, %rax
	ret

	.p2align 4
L(ExitHighCase3):
	cmp	$9, %r8
	je	L(StrncatExit9)
	cmp	$10, %r8
	je	L(StrncatExit10)
	cmp	$11, %r8
	je	L(StrncatExit11)
	cmp	$12, %r8
	je	L(StrncatExit12)
	cmp	$13, %r8
	je	L(StrncatExit13)
	cmp	$14, %r8
	je	L(StrncatExit14)
	cmp	$15, %r8
	je	L(StrncatExit15)
	movlpd	(%rcx), %xmm0
	movlpd	%xmm0, (%rdx)
	movlpd	8(%rcx), %xmm1
	movlpd	%xmm1, 8(%rdx)
	xor	%ah, %ah
	movb	%ah, 16(%rdx)
	mov	%rdi, %rax
	ret

	.p2align 4
L(StrncatExit0):
	mov	%rdi, %rax
	ret

	.p2align 4
L(StrncatExit15Bytes):
	cmp	$9, %r8
	je	L(StrncatExit9)
	cmpb	$0, 9(%rcx)
	jz	L(Exit10)
	cmp	$10, %r8
	je	L(StrncatExit10)
	cmpb	$0, 10(%rcx)
	jz	L(Exit11)
	cmp	$11, %r8
	je	L(StrncatExit11)
	cmpb	$0, 11(%rcx)
	jz	L(Exit12)
	cmp	$12, %r8
	je	L(StrncatExit12)
	cmpb	$0, 12(%rcx)
	jz	L(Exit13)
	cmp	$13, %r8
	je	L(StrncatExit13)
	cmpb	$0, 13(%rcx)
	jz	L(Exit14)
	cmp	$14, %r8
	je	L(StrncatExit14)
	movlpd	(%rcx), %xmm0
	movlpd	%xmm0, (%rdx)
	movlpd	7(%rcx), %xmm1
	movlpd	%xmm1, 7(%rdx)
	lea	14(%rdx), %rax
	cmpb	$1, (%rax)
	sbb	$-1, %rax
	xor	%cl, %cl
	movb	%cl, (%rax)
	mov	%rdi, %rax
	ret

	.p2align 4
L(StrncatExit8Bytes):
	cmpb	$0, (%rcx)
	jz	L(Exit1)
	cmp	$1, %r8
	je	L(StrncatExit1)
	cmpb	$0, 1(%rcx)
	jz	L(Exit2)
	cmp	$2, %r8
	je	L(StrncatExit2)
	cmpb	$0, 2(%rcx)
	jz	L(Exit3)
	cmp	$3, %r8
	je	L(StrncatExit3)
	cmpb	$0, 3(%rcx)
	jz	L(Exit4)
	cmp	$4, %r8
	je	L(StrncatExit4)
	cmpb	$0, 4(%rcx)
	jz	L(Exit5)
	cmp	$5, %r8
	je	L(StrncatExit5)
	cmpb	$0, 5(%rcx)
	jz	L(Exit6)
	cmp	$6, %r8
	je	L(StrncatExit6)
	cmpb	$0, 6(%rcx)
	jz	L(Exit7)
	cmp	$7, %r8
	je	L(StrncatExit7)
	movlpd	(%rcx), %xmm0
	movlpd	%xmm0, (%rdx)
	lea	7(%rdx), %rax
	cmpb	$1, (%rax)
	sbb	$-1, %rax
	xor	%cl, %cl
	movb	%cl, (%rax)
	mov	%rdi, %rax
	ret

# endif
END (STRCAT)
#endif
