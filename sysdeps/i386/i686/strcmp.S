/* Highly optimized version for ix86, x>=6.
   Copyright (C) 1999, 2000, 2002, 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1999.

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

#define PARMS	LINKAGE		/* no space for saved regs */
#define STR1	PARMS
#define STR2	STR1+PTR_SIZE

        .text
ENTRY (BP_SYM (strcmp))
	ENTER

	movl	STR1(%esp), %ecx
	movl	STR2(%esp), %edx
	CHECK_BOUNDS_LOW (%ecx, STR1(%esp))
	CHECK_BOUNDS_LOW (%edx, STR2(%esp))

L(oop):	movb	(%ecx), %al
	cmpb	(%edx), %al
	jne	L(neq)
	incl	%ecx
	incl	%edx
	testb	%al, %al
	jnz	L(oop)

	xorl	%eax, %eax
	/* when strings are equal, pointers rest one beyond
	   the end of the NUL terminators.  */
	CHECK_BOUNDS_HIGH (%ecx, STR1(%esp), jbe)
	CHECK_BOUNDS_HIGH (%edx, STR2(%esp), jbe)
	LEAVE
	ret

#ifndef __BOUNDED_POINTERS__
L(neq):	movl	$1, %eax
	movl	$-1, %ecx
	cmovbl	%ecx, %eax
#else
L(neq):	movl	$1, %eax
	ja	L(chk)
	negl	%eax
	/* When strings differ, pointers rest on
	   the unequal characters.  */
L(chk):	CHECK_BOUNDS_HIGH (%ecx, STR1(%esp), jb)
	CHECK_BOUNDS_HIGH (%edx, STR2(%esp), jb)
#endif

	LEAVE
	ret
END (BP_SYM (strcmp))
libc_hidden_builtin_def (strcmp)
