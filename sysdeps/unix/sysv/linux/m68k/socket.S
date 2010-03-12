/* Copyright (C) 1996, 1997, 1998, 2010 Free Software Foundation, Inc.
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

#include <sysdep-cancel.h>
#include <socketcall.h>

#define P(a, b) P2(a, b)
#define P2(a, b) a##b

	.text
/* The socket-oriented system calls are handled unusally in Linux.
   They are all gated through the single `socketcall' system call number.
   `socketcall' takes two arguments: the first is the subcode, specifying
   which socket function is being called; and the second is a pointer to
   the arguments to the specific function.

   The .S files for the other calls just #define socket and #include this.  */

#ifndef __socket
#ifndef NO_WEAK_ALIAS
#define __socket P(__,socket)
#else
#define __socket socket
#endif
#endif

.globl __socket
ENTRY (__socket)
#ifdef NEED_CANCELLATION
# if !defined CENABLE || !defined CDISABLE
#  error CENABLE and/or CDISABLE is not defined
# endif
	SINGLE_THREAD_P
	jne 1f
#endif

	/* Save registers.  */
	move.l %d2, %a0

	move.l #SYS_ify (socketcall), %d0 /* System call number in %d0.  */

	/* Use ## so `socket' is a separate token that might be #define'd.  */
	move.l #P (SOCKOP_,socket), %d1	/* Subcode is first arg to syscall.  */
	lea 4(%sp), %a1			/* Address of args is 2nd arg.  */
	move.l %a1, %d2

	/* Do the system call trap.  */
	trap #0

	/* Restore registers.  */
	move.l %a0, %d2

	/* %d0 is < 0 if there was an error.  */
	tst.l %d0
	jmi SYSCALL_ERROR_LABEL

	/* Successful; return the syscall's value.  */
	rts

#ifdef NEED_CANCELLATION
1:	cfi_startproc
	/* Enable asynchronous cancellation.  */
	CENABLE

	/* Save D2.  */
	move.l %d2, -(%sp)
	cfi_adjust_cfa_offset (4)
	cfi_rel_offset (%d2, 0)

	/* Save the result of CENABLE.  */
	move.l %d0, -(%sp)
	cfi_adjust_cfa_offset (4)

	/* Use ## so `socket' is a separate token that might be #define'd.  */
	move.l #P (SOCKOP_,socket), %d1	/* Subcode is first arg to syscall.  */
	lea 4+8(%sp), %a1		/* Address of args is 2nd arg.  */
	move.l %a1, %d2

	move.l #SYS_ify (socketcall), %d0 /* System call number in %d0.  */
	/* Do the system call trap.  */
	trap #0

	/* Restore cancellation.  */
	move.l %d0, %d2
	CDISABLE
	addq.l #4, %sp
	cfi_adjust_cfa_offset (-4)
	move.l %d2, %d0

	/* Restore registers.  */
	move.l (%sp)+, %d2
	cfi_adjust_cfa_offset (-4)
	cfi_restore (%d2)

	/* %d0 is < 0 if there was an error.  */
	tst.l %d0
	jmi SYSCALL_ERROR_LABEL
	cfi_endproc

	/* Successful; return the syscall's value.  */
	rts
#endif
	
PSEUDO_END (__socket)

#ifndef NO_WEAK_ALIAS
weak_alias (__socket, socket)
#endif
