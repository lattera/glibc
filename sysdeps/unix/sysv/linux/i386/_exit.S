/* Copyright (C) 2002 Free Software Foundation, Inc.
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

	.text
	.type	_exit,@function
	.global	_exit
_exit:
	movl	4(%esp), %ebx

	/* Try the new syscall first.  */
#ifdef __NR_exit_group
	movl	$__NR_exit_group, %eax
	int	$0x80
#endif

	/* Not available.  Now the old one.  */
	movl	$__NR_exit, %eax
	int	$0x80

	/* This must not fail.  Be sure we don't return.  */
	hlt
	.size	_exit,.-_exit

	.globl	_Exit
	.weak	_Exit
_Exit	= _exit
