/* Copyright (C) 2012-2016 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <lowlevellock.h>
#include <sysdep.h>

ENTRY(pthread_spin_lock)
1:	LOCK
	decl	0(%rdi)
	jne	2f
	xor	%eax, %eax
	ret

	.align	16
2:	rep
	nop
	cmpl	$0, 0(%rdi)
	jg	1b
	jmp	2b
END(pthread_spin_lock)
