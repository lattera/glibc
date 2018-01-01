/* MIPS syscall wrappers.
   Copyright (C) 2017-2018 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include <sysdep.h>
#include <sys/asm.h>

	.text
	.set	nomips16

/* long long __mips_syscall6 (long arg1, long arg2, long arg3, long arg4,
			      long arg5, long arg6,
			      long number)  */

ENTRY(__mips_syscall6)
	lw	v0, 24(sp)
	syscall
	move	v1, a3
	jr	ra
END(__mips_syscall6)
libc_hidden_def (__mips_syscall6)
