/* Highly optimized version for ix86, x>=6.
   Copyright (C) 1999 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1999.

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
   s1          (sp + 4)
   s2          (sp + 8)
*/


        .text
ENTRY (strcmp)
	movl	4(%esp), %ecx
	movl	8(%esp), %edx
1:	movb	(%ecx), %al
	incl	%ecx
	cmpb	(%edx), %al
	jne	2f
	incl	%edx
	testb	%al, %al
	jnz	1b
	xorl	%eax, %eax
	jmp	3f
2:	movl	$1, %eax
	ja	3f
	negl	%eax
3:	ret
END (strcmp)
