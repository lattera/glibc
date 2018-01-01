/* Copyright (C) 2002-2018 Free Software Foundation, Inc.
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

	.text
	.type	_exit,@function
	.global	_exit
_exit:
	movl	4(%esp), %ebx

	/* Try the new syscall first.  */
#ifdef __NR_exit_group
	movl	$__NR_exit_group, %eax
	ENTER_KERNEL
#endif

	/* Not available.  Now the old one.  */
	movl	$__NR_exit, %eax
	/* Don't bother using ENTER_KERNEL here.  If the exit_group
	   syscall is not available AT_SYSINFO isn't either.  */
	int	$0x80

	/* This must not fail.  Be sure we don't return.  */
	hlt
	.size	_exit,.-_exit

libc_hidden_def (_exit)
rtld_hidden_def (_exit)
weak_alias (_exit, _Exit)
