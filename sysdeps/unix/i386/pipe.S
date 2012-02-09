/* Copyright (C) 1991,92,93,95,97,2002 Free Software Foundation, Inc.
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

#include <sysdep.h>

SYSCALL__ (pipe, 1)
	movl 4(%esp), scratch
	movl %eax, (scratch)
	movl r1, 4(scratch)
	xorl %eax, %eax
	ret
PSEUDO_END (__pipe)

libc_hidden_def (__pipe)
weak_alias (__pipe, pipe)
