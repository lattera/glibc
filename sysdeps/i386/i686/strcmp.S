/* Highly optimized version for ix86, x>=6.
   Copyright (C) 1999, 2000 Free Software Foundation, Inc.
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
#include "bp-asm.h"

#define PARMS	LINKAGE		/* no space for saved regs */
#define STR1	PARMS
#define STR2	STR1+PTR_SIZE

        .text
ENTRY (strcmp)
	ENTER

	movl	STR1(%esp), %ecx
	movl	STR2(%esp), %edx

L(oop):	movb	(%ecx), %al
	incl	%ecx
	cmpb	(%edx), %al
	jne	L(neq)
	incl	%edx
	testb	%al, %al
	jnz	L(oop)

	xorl	%eax, %eax
	jmp	L(out)

L(neq):	movl	$1, %eax
	ja	L(out)
	negl	%eax

L(out):	LEAVE
	ret
END (strcmp)
