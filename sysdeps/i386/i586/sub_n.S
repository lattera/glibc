/* Pentium __mpn_sub_n -- Subtract two limb vectors of the same length > 0
   and store difference in a third limb vector.

Copyright (C) 1992, 1994, 1995 Free Software Foundation, Inc.

This file is part of the GNU MP Library.

The GNU MP Library is free software; you can redistribute it and/or modify
it under the terms of the GNU Library General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at your
option) any later version.

The GNU MP Library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
License for more details.

You should have received a copy of the GNU Library General Public License
along with the GNU MP Library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA. */

/*
   INPUT PARAMETERS
   res_ptr	(sp + 4)
   s1_ptr	(sp + 8)
   s2_ptr	(sp + 12)
   size		(sp + 16)
*/

#define r1	%eax
#define r2	%edx
#define src1	%esi
#define src2	%ebp
#define dst	%edi
#define x	%ebx

#include "sysdep.h"
#include "asm-syntax.h"

.text
	ALIGN (3)
	.globl C_SYMBOL_NAME(__mpn_sub_n)
C_SYMBOL_NAME(__mpn_sub_n:)
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	pushl	%ebp

	movl	20(%esp),dst		/* res_ptr */
	movl	24(%esp),src1		/* s1_ptr */
	movl	28(%esp),src2		/* s2_ptr */
	movl	32(%esp),%ecx		/* size */

	movl	(src2),x

	decl	%ecx
	movl	%ecx,r2
	shrl	$3,%ecx
	andl	$7,r2
	testl	%ecx,%ecx		/* zero carry flag */
	jz	Lend
	pushl	r2

	ALIGN (3)
Loop:	movl	28(dst),%eax		/* fetch destination cache line */
	leal	32(dst),dst

L1:	movl	(src1),r1
	movl	4(src1),r2
	sbbl	x,r1
	movl	4(src2),x
	sbbl	x,r2
	movl	8(src2),x
	movl	r1,-32(dst)
	movl	r2,-28(dst)

L2:	movl	8(src1),r1
	movl	12(src1),r2
	sbbl	x,r1
	movl	12(src2),x
	sbbl	x,r2
	movl	16(src2),x
	movl	r1,-24(dst)
	movl	r2,-20(dst)

L3:	movl	16(src1),r1
	movl	20(src1),r2
	sbbl	x,r1
	movl	20(src2),x
	sbbl	x,r2
	movl	24(src2),x
	movl	r1,-16(dst)
	movl	r2,-12(dst)

L4:	movl	24(src1),r1
	movl	28(src1),r2
	sbbl	x,r1
	movl	28(src2),x
	sbbl	x,r2
	movl	32(src2),x
	movl	r1,-8(dst)
	movl	r2,-4(dst)

	leal	32(src1),src1
	leal	32(src2),src2
	decl	%ecx
	jnz	Loop

	popl	r2
Lend:
	decl	r2			/* test r2 w/o clobbering carry */
	js	Lend2
	incl	r2
Loop2:
	leal	4(dst),dst
	movl	(src1),r1
	sbbl	x,r1
	movl	4(src2),x
	movl	r1,-4(dst)
	leal	4(src1),src1
	leal	4(src2),src2
	decl	r2
	jnz	Loop2
Lend2:
	movl	(src1),r1
	sbbl	x,r1
	movl	r1,(dst)

	sbbl	%eax,%eax
	negl	%eax

	popl	%ebp
	popl	%ebx
	popl	%esi
	popl	%edi
	ret
