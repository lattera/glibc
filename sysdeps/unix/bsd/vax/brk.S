/* Copyright (C) 1991, 1995, 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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

#ifndef	SYS_brk
#define	SYS_brk	17
#endif

#ifndef	HAVE_GNU_LD
#define	__end	_end
#endif

.data
.globl ___curbrk
___curbrk:
	.long __end

.text
ENTRY (__brk)
	cmpl 4(ap), __end
	bgeq 0f
	movl __env, 4(ap)
0:	chmk $SYS_brk
	bcs 1f
	movl 4(ap), ___curbrk
	clrl r0
	ret
1:
	jmp syscall_error

weak_alias (__brk, brk)
