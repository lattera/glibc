/* memset/bzero -- set memory area to CH/0
Highly optimized version for ix85, x>=5.
Copyright (C) 1996 Free Software Foundation, Inc.
This file is part of the GNU C Library.
Contributed by Torbjorn Granlund, <tege@matematik.su.se>

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

#include <sysdep.h>

/*
   INPUT PARAMETERS:
   (memset)			(bzero)
   dst          (sp + 4)	dst	(sp + 4)
   ch           (sp + 8)	len	(sp + 8)
   len		(sp + 12)
*/


        .text
ENTRY (memset)
	pushl	%edi

	movl	8(%esp), %edi	/* destination pointer */
#ifdef memset
	xorl	%eax, %eax	/* we fill with 0 */
	movl	12(%esp), %edx	/* size (in 8-bit words) */
#else
	movb	12(%esp), %al	/* use CH to fill */
	movl	16(%esp), %edx	/* size (in 8-bit words) */

	movb	%al, %ah
	movl	%eax, %ecx
	shll	$16, %eax
	movw	%cx, %ax
#endif
	cld

/* If less than 36 bytes to write, skip tricky code (it wouldn't work).  */
	cmpl	$36, %edx
	movl	%edx, %ecx	/* needed when branch is taken! */
	jl	L2

/* First write 0-3 bytes to make the pointer 32-bit aligned.  */
	movl	%edi, %ecx	/* Copy ptr to ecx... */
	negl	%ecx		/* ...and negate that and... */
	andl	$3, %ecx	/* ...mask to get byte count.  */
	subl	%ecx, %edx	/* adjust global byte count */
	rep
	stosb

	subl	$32, %edx	/* offset count for unrolled loop */
	movl	(%edi), %ecx	/* Fetch destination cache line */

	.align	2, 0x90		/* supply 0x90 for broken assemblers */
L1:	movl	28(%edi), %ecx	/* allocate cache line for destination */
	subl	$32, %edx	/* decr loop count */
	movl	%eax, 0(%edi)	/* store words pairwise */
	movl	%eax, 4(%edi)
	movl	%eax, 8(%edi)
	movl	%eax, 12(%edi)
	movl	%eax, 16(%edi)
	movl	%eax, 20(%edi)
	movl	%eax, 24(%edi)
	movl	%eax, 28(%edi)
	leal	32(%edi), %edi	/* update destination pointer */
	jge	L1

	leal	32(%edx), %ecx	/* reset offset count */

/* Write last 0-7 full 32-bit words (up to 8 words if loop was skipped).  */
L2:	shrl	$2, %ecx	/* convert byte count to longword count */
	rep
	stosl

/* Finally write the last 0-3 bytes.  */
	movl	%edx, %ecx
	andl	$3, %ecx
	rep
	stosb

	/* Load result (only if used as memset).  */
#ifndef memset
	movl	8(%esp), %eax
#endif
	popl	%edi

	ret
END (memset)
