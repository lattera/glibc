/* memset/bzero -- set memory area to CH/0
   Highly optimized version for ix86, x>=6.
   Copyright (C) 1999 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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

#include <sysdep.h>
#include "asm-syntax.h"

/*
   INPUT PARAMETERS:
   (memset)			(bzero)
   dst          (sp + 4)	dst	(sp + 4)
   ch           (sp + 8)	len	(sp + 8)
   len		(sp + 12)
*/


        .text
ENTRY (memset)
	cld
	pushl	%edi
	movl	8(%esp), %edx
#ifdef memset
	movl	12(%esp), %ecx
	xorl	%eax, %eax
#else
	movzbl	12(%esp), %eax
	movl	16(%esp), %ecx
#endif
	jecxz	1f
	movl	%edx, %edi
	andl	$3, %edx
	jz	2f
	jnp	3f
	stosb
	decl	%ecx
	jz	1f
3:	stosb
	decl	%ecx
	jz	1f
	xorl	$3, %edx
	jz	2f
	stosb
	decl	%ecx
2:	movl	%ecx, %edx
	shrl	$2, %ecx
	andl	$3, %edx
#ifndef memset
	imul	$0x01010101, %eax
#endif
	rep
	stosl
	movl	%edx, %ecx
	rep
	stosb

1:	movl	8(%esp), %eax
	popl	%edi
	ret
END (memset)
