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
	stwu 1,-32(1)
	stw 31,16(1)
	stw 30,20(1)

	/* Check for child_stack == NULL || fn == NULL.  */
	cmpwi 0,4,0
	cmpwi 1,3,0
	cror 2+0*4,2+0*4,2+1*4
	beq- 0,badargs

	/* Set up stack frame for child.  */
	addi 4,4,-16
	clrrwi 4,4,4
	li 0,0
	stw 0,0(4)

	/* Save new stack, fn, args across syscall.  */
	mr 30,3		/* Function in r30.  */
	mr 31,6		/* Arguments in r31.  */

	/* 'flags' argument is first parameter to clone syscall. (The other
	   argument is the stack pointer, already in r4.)  */
	mr 3,5

	/* Do the call.  */
	DO_CALL(SYS_ify(clone))
	bso- error
	beq child

	/* Parent.  Restore registers & return.  */
	lwz 31,16(1)
	lwz 30,20(1)
	addi 1,1,32
	blr

child:
	/* Call procedure.  */
	mtlr 30
	mr 3,31
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
