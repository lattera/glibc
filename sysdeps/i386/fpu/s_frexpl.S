/* ix87 specific frexp implementation for long double.
   Copyright (C) 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1997.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <machine/asm.h>

#ifdef __ELF__
	.section .rodata
#else
	.text
#endif

	.align ALIGNARG(4)
	ASM_TYPE_DIRECTIVE(two64,@object)
two64:	.byte 0, 0, 0, 0, 0, 0, 0xf0, 0x43
	ASM_SIZE_DIRECTIVE(two64)

#ifdef PIC
#define MO(op) op##@GOTOFF(%edx)
#else
#define MO(op) op
#endif

	.text
ENTRY(__frexpl)
	movl	4(%esp), %ecx
	movl	12(%esp), %eax
	orl	8(%esp), %ecx
	movl	%eax, %edx
	andl	$0x7fff, %eax
	orl	%eax, %ecx
	jz	1f
	xorl	%ecx, %ecx
	cmpl	$0x7fff, %eax
	je	1f

	cmpl	$0, %eax
	je	2f

	fldt	4(%esp)
#ifdef	PIC
	call	3f
3:	popl	%edx
	addl	$_GLOBAL_OFFSET_TABLE_+[.-3b], %edx
#endif
	fmull	MO(two64)	/* It's not necessary to use a 80bit factor */
	movl	$-64, %ecx
	fstpt	4(%esp)
	movl	12(%esp), %eax
	movl	%eax, %edx
	andl	$0x7fff, %eax

2:	andl	$0x8000, %edx
	subl	$16382, %eax
	orl	$0x3ffe, %edx
	addl	%eax, %ecx
	movl	%edx, 12(%esp)

	/* Store %ecx in the variable pointed to by the second argument,
	   get the factor from the stack and return.  */
1:	movl	16(%esp), %eax
	fldt	4(%esp)
	movl	%ecx, (%eax)
	ret
END(__frexpl)
weak_alias (__frexpl, frexpl)
