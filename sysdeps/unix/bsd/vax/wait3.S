/* Copyright (C) 1991, 1992, 1995 Free Software Foundation, Inc.
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

ENTRY(___wait3)
	movel 8(ap), r1
	movel 12(ap), r0
	/* Set all condition codes to tell the kernel this is wait3.  */
	bispsw $15
	chmk $SYS_wait
	bcs error

	movl 4(ap), r2
	beq 1f
	movl r1, (r2)
1:	ret

.globl	syscall_error
error:	jmp syscall_error

weak_alias (__wait3, wait3)
