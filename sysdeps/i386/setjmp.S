/* setjmp for i386.
Copyright (C) 1995 Free Software Foundation, Inc.
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
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <sysdep.h>
#define _ASM
#include <jmp_buf.h>

ENTRY (__sigsetjmp)
	movl 4(%esp), %eax	/* User's jmp_buf in %eax.  */
     	/* Save registers.  */
	movl %ebx, JB_BX(%eax)
	movl %esi, JB_SI(%eax)
	movl %edi, JB_DI(%eax)
	movl %ebp, JB_BP(%eax)
	leal 4(%esp), %ecx	/* Save SP as it will be after we return.  */
     	movl %ecx, JB_SP(%eax)
	movl 0(%esp), %ecx	/* Save PC we are returning to now.  */
     	movl %ecx, JB_PC(%eax)
	xorl %eax, %eax		/* Return zero.  */
	ret
