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

ENTRY(__clone)
	/* Set up stack frame, save registers.  */
	stwu  %r1,-32(%r1)
	stw   %r31,16(%r1)
	stw   %r30,20(%r1)

	/* Check for child_stack == NULL || fn == NULL.  */
	cmpwi %cr0,%r4,0
	beq-  %cr0,badargs
	cmpwi %cr1,%r3,0
	beq-  %cr1,badargs

	/* Set up stack frame for child.  */
	addi  %r4,%r4,-16
	clrrwi %r4,%r4,4
	li    %r0,0
	stw   %r0,0(%r4)

	/* Save new stack, fn, args across syscall.  */
	mr    %r30,%r3		/* Function in r30.  */
	mr    %r31,%r6		/* Arguments in r31.  */

	/* 'flags' argument is first parameter to clone syscall. (The other
	   argument is the stack pointer, already in r4.)  */
	mr    %r3,%r5

	/* Do the call.  */
	DO_CALL(SYS_ify(clone))
	bso-  error
	beq   child

	/* Parent.  Restore registers & return.  */
	lwz   %r31,16(%r1)
	lwz   %r30,20(%r1)
	addi  %r1,%r1,32
	blr

child:
	/* Call procedure.  */
	mtlr  %r30
	mr    %r3,%r31
	blrl
	/* Call _exit with result from procedure.  */
#ifdef PIC
	b _exit@plt
#else
	b _exit
#endif

badargs:
	li 3,-EINVAL
error:
#ifdef PIC
	b __syscall_error@plt
#else
	b __syscall_error
#endif

PSEUDO_END (__clone)

weak_alias (__clone, clone)
