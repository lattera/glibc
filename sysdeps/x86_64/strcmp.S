/* Highly optimized version for x86-64.
   Copyright (C) 1999, 2000, 2002, 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Based on i686 version contributed by Ulrich Drepper
   <drepper@cygnus.com>, 1999.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <sysdep.h>
#include "asm-syntax.h"
#include "bp-sym.h"
#include "bp-asm.h"

        .text
ENTRY (BP_SYM (strcmp))
L(oop):	movb	(%rdi), %al
	cmpb	(%rsi), %al
	jne	L(neq)
	incq	%rdi
	incq	%rsi
	testb	%al, %al
	jnz	L(oop)

	xorq	%rax, %rax
	ret

L(neq):	movl	$1, %eax
	movl	$-1, %ecx
	cmovbl	%ecx, %eax
	ret
END (BP_SYM (strcmp))
libc_hidden_builtin_def (strcmp)
