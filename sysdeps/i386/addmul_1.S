/* i80386 __mpn_addmul_1 -- Multiply a limb vector with a limb and add
   the result to a second limb vector.
   Copyright (C) 1992, 1994, 1997, 1998, 2000 Free Software Foundation, Inc.
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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
   MA 02111-1307, USA. */

#include "sysdep.h"
#include "asm-syntax.h"
#include "bp-asm.h"

#define PARMS	LINKAGE+16	/* space for 4 saved regs */
#define RES	PARMS
#define S1	RES+PTR_SIZE
#define SIZE	S1+PTR_SIZE
#define S2LIMB	SIZE+4

#define res_ptr edi
#define s1_ptr esi
#define sizeP ecx
#define s2_limb ebp

	.text
ENTRY(__mpn_addmul_1)
	ENTER

	pushl	%edi
	pushl	%esi
	pushl	%ebx
	pushl	%ebp

	movl	RES(%esp), %res_ptr
	movl	S1(%esp), %s1_ptr
	movl	SIZE(%esp), %sizeP
	movl	S2LIMB(%esp), %s2_limb

	leal	(%res_ptr,%sizeP,4), %res_ptr
	leal	(%s1_ptr,%sizeP,4), %s1_ptr
	negl	%sizeP
	xorl	%ebx, %ebx
	ALIGN (3)
L(oop):
	movl	(%s1_ptr,%sizeP,4), %eax
	mull	%s2_limb
	addl	%ebx, %eax
	adcl	$0, %edx
	addl	%eax, (%res_ptr,%sizeP,4)
	adcl	$0, %edx
	movl	%edx, %ebx

	incl	%sizeP
	jnz	L(oop)
	movl	%ebx, %eax

	popl	%ebp
	popl	%ebx
	popl	%esi
	popl	%edi

	LEAVE
	ret
END(__mpn_addmul_1)
