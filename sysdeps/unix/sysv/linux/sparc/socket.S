/* Copyright (C) 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Miguel de Icaza <miguel@gnu.ai.mit.edu>, 1997.

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
#include <sys/socketcall.h>

#define P(a, b) P2(a, b)
#define P2(a, b) a##b

	.text
/* The socket-oriented system calls are handled unusually in Linux.
   They are all gated through the single `socketcall' system call number.
   `socketcall' takes two arguments: the first is the subcode, specifying
   which socket function is being called; and the second is a pointer to
   the arguments to the specific function.

   The .S files for the other calls just #define socket and #include this.  */

.globl P(__,socket)
ENTRY (P(__,socket))
	mov SYS_ify(socketcall), %g1    /* System call number */

	/* Use ## so `socket' is a separate token that might be #define'd.  */
	mov P(SOCKOP_,socket), %o0	/* Subcode is first arg to syscall.  */
	mov %i0,%o1		        /* args pointer is second arg to syscall */

	t 0x10
	bcc,a	1
	nop
	save %sp,96,%sp
	call __errno_location
	nop
	st %i0,[%o0]
	restore
	retl
	mov -1,%o0
1:
	ret

PSEUDO_END (P(__,socket))

weak_alias (P(__,socket), socket)
