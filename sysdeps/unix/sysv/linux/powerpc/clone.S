/* Wrapper around clone system call.
   Copyright (C) 1997 Free Software Foundation, Inc.
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
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <sysdep.h>
#define _ERRNO_H	1
#include <errnos.h>

/* This is the only really unusual system call in PPC linux, but not
   because of any weirdness in the system call itself; because of
   all the freaky stuff we have to do to make the call useful.  */

/* int clone(int (*fn)(void *arg), void *child_stack, int flags, void *arg); */

ENTRY(clone)
	/* Set up stack frame, save registers.  */
	stwu 1,-20(1)
	stw 31,16(1)

	/* Check for child_stack == NULL, fn == NULL.  */
	mr. 31,4
	cmpwi 1,3,0
	cror 2+0*4,2+0*4,2+1*4
	beq- 0,badargs

	/* Save 'fn' and its argument on the new stack.  */
	stw 3,0(4)
	stw 6,4(4)

	/* 'flags' argument is (only) parameter to clone syscall.  */
	mr 3,5

	/* Do the call.  */
	DO_CALL(SYS_ify(clone))
	bso- error
	beq child

	/* Parent.  Restore registers & return.  */
	lwz 31,20(1)
	addi 1,1,20
	blr

child:
	/* Get address of procedure to call.  */
	lwz 0,0(31)
	/* Set up argument register.  */
	lwz 3,4(31)
	mtlr 0
	/* Switch to new stack.  */
	mr 1,31
	/* Call procedure.  */
	blrl
	/* Call _exit with result from procedure.  */
	DO_CALL (SYS_ify (exit))

badargs:
	li 3,-EINVAL
error:
#ifdef PIC
	b __syscall_error@plt
#else
	b __syscall_error
#endif
