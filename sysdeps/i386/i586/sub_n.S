/* Pentium __mpn_sub_n -- Subtract two limb vectors of the same length > 0
   and store difference in a third limb vector.
   Copyright (C) 1992, 94, 95, 96, 97, 98, 2000 Free Software Foundation, Inc.
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

#define PARMS	LINKAGE+16		/* space for 4 saved regs */
#define RES	PARMS
#define S1	RES+PTR_SIZE
#define S2	S1+PTR_SIZE
#define SIZE	S2+PTR_SIZE

	.text
ENTRY(__mpn_sub_n)
	ENTER

	pushl	%edi
	pushl	%esi
	pushl	%ebx
	pushl	%ebp

	movl	RES(%esp),%edi
	movl	S1(%esp),%esi
	movl	S2(%esp),%ebp
	movl	SIZE(%esp),%ecx

	movl	(%ebp),%ebx

	decl	%ecx
	movl	%ecx,%edx
	shrl	$3,%ecx
	andl	$7,%edx
	testl	%ecx,%ecx		/* zero carry flag */
	jz	L(end)
	pushl	%edx

	ALIGN (3)
L(oop):	movl	28(%edi),%eax		/* fetch destination cache line */
	leal	32(%edi),%edi

L(1):	movl	(%esi),%eax
	movl	4(%esi),%edx
	sbbl	%ebx,%eax
	movl	4(%ebp),%ebx
	sbbl	%ebx,%edx
	movl	8(%ebp),%ebx
	movl	%eax,-32(%edi)
	movl	%edx,-28(%edi)

L(2):	movl	8(%esi),%eax
	movl	12(%esi),%edx
	sbbl	%ebx,%eax
	movl	12(%ebp),%ebx
	sbbl	%ebx,%edx
	movl	16(%ebp),%ebx
	movl	%eax,-24(%edi)
	movl	%edx,-20(%edi)

L(3):	movl	16(%esi),%eax
	movl	20(%esi),%edx
	sbbl	%ebx,%eax
	movl	20(%ebp),%ebx
	sbbl	%ebx,%edx
	movl	24(%ebp),%ebx
	movl	%eax,-16(%edi)
	movl	%edx,-12(%edi)

L(4):	movl	24(%esi),%eax
	movl	28(%esi),%edx
	sbbl	%ebx,%eax
	movl	28(%ebp),%ebx
	sbbl	%ebx,%edx
	movl	32(%ebp),%ebx
	movl	%eax,-8(%edi)
	movl	%edx,-4(%edi)

	leal	32(%esi),%esi
	leal	32(%ebp),%ebp
	decl	%ecx
	jnz	L(oop)

	popl	%edx
L(end):
	decl	%edx			/* test %edx w/o clobbering carry */
	js	L(end2)
	incl	%edx
L(oop2):
	leal	4(%edi),%edi
	movl	(%esi),%eax
	sbbl	%ebx,%eax
	movl	4(%ebp),%ebx
	movl	%eax,-4(%edi)
	leal	4(%esi),%esi
	leal	4(%ebp),%ebp
	decl	%edx
	jnz	L(oop2)
L(end2):
	movl	(%esi),%eax
	sbbl	%ebx,%eax
	movl	%eax,(%edi)

	sbbl	%eax,%eax
	negl	%eax

	popl	%ebp
	popl	%ebx
	popl	%esi
	popl	%edi

	LEAVE
	ret
END(__mpn_sub_n)
