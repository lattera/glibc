/* Copy memory block and return pointer to following byte.
   For Intel 80x86, x>=6.
   This file is part of the GNU C Library.
   Copyright (C) 1998 Free Software Foundation, Inc.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1998.

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

/* strtok:
	INPUT PARAMETER:
	dest		(sp + 4)
	src		(sp + 8)
	len		(sp + 12)
*/

ENTRY(__mempcpy)
	movl	%edi, %eax
	movl	4(%esp), %edi
	movl	%esi, %edx
	movl	8(%esp), %esi
	movl	12(%esp), %ecx
	cld
	shrl	$1, %ecx
	jnc	1f
	movsb
1:	shrl	$1, %ecx
	jnc	2f
	movsw
2:	rep
	movsl
	xchgl	%edi, %eax
	movl	%edx, %esi
	ret
END(mempcpy)
weak_alias (__mempcpy, mempcpy)
