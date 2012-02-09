/* strcpy/stpcpy checking implementation for x86-64.
   Copyright (C) 2002, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Andreas Jaeger <aj@suse.de>, 2002.
   Adopted into checking version by Jakub Jelinek <jakub@redhat.com>.

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
#include "asm-syntax.h"
#include "bp-sym.h"
#include "bp-asm.h"

#ifndef USE_AS_STPCPY_CHK
# define STRCPY_CHK __strcpy_chk
#endif

	.text
ENTRY (STRCPY_CHK)
	movq	%rsi, %rcx	/* Source register. */
	andl	$7, %ecx	/* mask alignment bits */
#ifndef USE_AS_STPCPY_CHK
	movq	%rdi, %r10	/* Duplicate destination pointer.  */
#endif
	jz 5f			/* aligned => start loop */

	cmpq	$8, %rdx	/* Check if only few bytes left in
				   destination.  */
	jb	50f

	subq	$8, %rcx	/* We need to align to 8 bytes.  */
	addq	%rcx, %rdx	/* Subtract count of stored bytes
				   in the cycle below from destlen.  */

	/* Search the first bytes directly.  */
0:
	movb	(%rsi), %al	/* Fetch a byte */
	testb	%al, %al	/* Is it NUL? */
	movb	%al, (%rdi)	/* Store it */
	jz	4f		/* If it was NUL, done! */
	incq	%rsi
	incq	%rdi
	incl	%ecx
	jnz	0b

5:
	movq $0xfefefefefefefeff,%r8
	cmpq	$32, %rdx	/* Are there enough bytes in destination
				   for the next unrolled round?  */
	jb	60f		/* If not, avoid the unrolled loop.  */

	/* Now the sources is aligned.  Unfortunatly we cannot force
	   to have both source and destination aligned, so ignore the
	   alignment of the destination.  */
	.p2align 4
1:
	/* 1st unroll.  */
	movq	(%rsi), %rax	/* Read double word (8 bytes).  */
	addq	$8, %rsi	/* Adjust pointer for next word.  */
	movq	%rax, %r9	/* Save a copy for NUL finding.  */
	addq	%r8, %r9	/* add the magic value to the word.  We get
				   carry bits reported for each byte which
				   is *not* 0 */
	jnc	3f		/* highest byte is NUL => return pointer */
	xorq	%rax, %r9	/* (word+magic)^word */
	orq	%r8, %r9	/* set all non-carry bits */
	incq	%r9		/* add 1: if one carry bit was *not* set
				   the addition will not result in 0.  */

	jnz	3f		/* found NUL => return pointer */

	movq	%rax, (%rdi)	/* Write value to destination.  */
	addq	$8, %rdi	/* Adjust pointer.  */

	/* 2nd unroll.  */
	movq	(%rsi), %rax	/* Read double word (8 bytes).  */
	addq	$8, %rsi	/* Adjust pointer for next word.  */
	movq	%rax, %r9	/* Save a copy for NUL finding.  */
	addq	%r8, %r9	/* add the magic value to the word.  We get
				   carry bits reported for each byte which
				   is *not* 0 */
	jnc	3f		/* highest byte is NUL => return pointer */
	xorq	%rax, %r9	/* (word+magic)^word */
	orq	%r8, %r9	/* set all non-carry bits */
	incq	%r9		/* add 1: if one carry bit was *not* set
				   the addition will not result in 0.  */

	jnz	3f		/* found NUL => return pointer */

	movq	%rax, (%rdi)	/* Write value to destination.  */
	addq	$8, %rdi	/* Adjust pointer.  */

	/* 3rd unroll.  */
	movq	(%rsi), %rax	/* Read double word (8 bytes).  */
	addq	$8, %rsi	/* Adjust pointer for next word.  */
	movq	%rax, %r9	/* Save a copy for NUL finding.  */
	addq	%r8, %r9	/* add the magic value to the word.  We get
				   carry bits reported for each byte which
				   is *not* 0 */
	jnc	3f		/* highest byte is NUL => return pointer */
	xorq	%rax, %r9	/* (word+magic)^word */
	orq	%r8, %r9	/* set all non-carry bits */
	incq	%r9		/* add 1: if one carry bit was *not* set
				   the addition will not result in 0.  */

	jnz	3f		/* found NUL => return pointer */

	movq	%rax, (%rdi)	/* Write value to destination.  */
	addq	$8, %rdi	/* Adjust pointer.  */

	/* 4th unroll.  */
	movq	(%rsi), %rax	/* Read double word (8 bytes).  */
	addq	$8, %rsi	/* Adjust pointer for next word.  */
	movq	%rax, %r9	/* Save a copy for NUL finding.  */
	addq	%r8, %r9	/* add the magic value to the word.  We get
				   carry bits reported for each byte which
				   is *not* 0 */
	jnc	3f		/* highest byte is NUL => return pointer */
	xorq	%rax, %r9	/* (word+magic)^word */
	orq	%r8, %r9	/* set all non-carry bits */
	incq	%r9		/* add 1: if one carry bit was *not* set
				   the addition will not result in 0.  */

	jnz	3f		/* found NUL => return pointer */

	subq	$32, %rdx	/* Adjust destlen.  */
	movq	%rax, (%rdi)	/* Write value to destination.  */
	addq	$8, %rdi	/* Adjust pointer.  */
	cmpq	$32, %rdx	/* Are there enough bytes in destination
				   for the next unrolled round?  */
	jae	1b		/* Next iteration.  */

60:
	cmpq	$8, %rdx	/* Are there enough bytes in destination
				   for the next unrolled round?  */
	jb	50f		/* Now, copy and check byte by byte.  */

	movq	(%rsi), %rax	/* Read double word (8 bytes).  */
	addq	$8, %rsi	/* Adjust pointer for next word.  */
	movq	%rax, %r9	/* Save a copy for NUL finding.  */
	addq	%r8, %r9	/* add the magic value to the word.  We get
				   carry bits reported for each byte which
				   is *not* 0 */
	jnc	3f		/* highest byte is NUL => return pointer */
	xorq	%rax, %r9	/* (word+magic)^word */
	orq	%r8, %r9	/* set all non-carry bits */
	incq	%r9		/* add 1: if one carry bit was *not* set
				   the addition will not result in 0.  */

	jnz	3f		/* found NUL => return pointer */

	subq	$8, %rdx	/* Adjust destlen.  */
	movq	%rax, (%rdi)	/* Write value to destination.  */
	addq	$8, %rdi	/* Adjust pointer.  */
	jmp	60b		/* Next iteration.  */

	/* Do the last few bytes. %rax contains the value to write.
	   The loop is unrolled twice.  */
	.p2align 4
3:
	/* Note that stpcpy needs to return with the value of the NUL
	   byte.  */
	movb	%al, (%rdi)	/* 1st byte.  */
	testb	%al, %al	/* Is it NUL.  */
	jz	4f		/* yes, finish.  */
	incq	%rdi		/* Increment destination.  */
	movb	%ah, (%rdi)	/* 2nd byte.  */
	testb	%ah, %ah	/* Is it NUL?.  */
	jz	4f		/* yes, finish.  */
	incq	%rdi		/* Increment destination.  */
	shrq	$16, %rax	/* Shift...  */
	jmp	3b		/* and look at next two bytes in %rax.  */

51:
	/* Search the bytes directly, checking for overflows.  */
	incq	%rsi
	incq	%rdi
	decq	%rdx
	jz	HIDDEN_JUMPTARGET (__chk_fail)
52:
	movb	(%rsi), %al	/* Fetch a byte */
	testb	%al, %al	/* Is it NUL? */
	movb	%al, (%rdi)	/* Store it */
	jnz	51b		/* If it was NUL, done! */
4:
#ifdef USE_AS_STPCPY_CHK
	movq	%rdi, %rax	/* Destination is return value.  */
#else
	movq	%r10, %rax	/* Source is return value.  */
#endif
	retq

50:
	testq	%rdx, %rdx
	jnz	52b
	jmp	HIDDEN_JUMPTARGET (__chk_fail)

END (STRCPY_CHK)
