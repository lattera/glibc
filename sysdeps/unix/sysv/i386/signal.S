/* Copyright (C) 1992 Free Software Foundation, Inc.
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

/* This is just a standard system call, except we need to load %edx
   with the address of the `__sigreturn' function.  */

	.globl syscall_error
	.globl C_SYMBOL_NAME(__sigreturn)
ENTRY (signal)
	lea SYS_signal, %eax
	lea C_SYMBOL_NAME(__sigreturn), %edx
	.byte 0x9a, 0, 0, 0, 0, 7, 0 /* lcall $7, $0 -- GAS bug.  */
	jb syscall_error
	ret
