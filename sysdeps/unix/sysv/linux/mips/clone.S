/* Copyright (C) 1996, 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ralf Baechle <ralf@gnu.ai.mit.edu>, 1996.

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

/* clone() is even more special than fork() as it mucks with stacks
   and invokes a function in the right context after its all over.  */

#include <sys/asm.h>
#include <asm/unistd.h>
#include <sysdep.h>
#define _ERRNO_H	1
#include <bits/errno.h>

/* int clone(int (*fn)(), void *child_stack, int flags, int nargs, ...) */

#define FRAMESZ  4*SZREG
#if _MIPS_SIM == _MIPS_SIM_ABI32
#define MAX_REG_ARGS 4
#else
#define MAX_REG_ARGS 6
#endif

	.text
NESTED(__clone,4*SZREG,sp)
#ifdef PIC
	.set		noreorder
	.cpload		$25
	.set		reorder
	.cprestore	16
#endif
	PTR_SUBIU	sp,FRAMESZ
#ifdef PROF
	.set		noat
	move		$1,ra
	jal		_mcount
	.set		at
#endif

	/* Sanity check arguments.  */
	li		v0,EINVAL
	beqz		a0,error	/* no NULL function pointers */
	beqz		a1,error	/* no NULL stack pointers */
	bltz		a3,error	/* no negative argument counts */

	/* Allocate space on the new stack and copy args over */
	move		t0,a3		# save nargs for __thread_start
	PTR_SLL		t1,a3,PTR_SCALESHIFT
	PTR_ADDU	t1,a3,sp
1:	REG_L		t2,-SZREG(t1)
	PTR_SUBIU	t1,SZREG
	REG_S		t2,-SZREG(a1)
	PTR_SUBIU	a3,1
	PTR_SUBIU	a1,SZREG
	bnez		a3,1b

	/* Do the system call */
	move		t9,a0		# get fn ptr out of the way
	move		a0,a2
	li		v0,__NR_clone
	syscall

	bnez		a3,error
	beqz		v0,__thread_start

	/* Successful return from the parent */
	PTR_ADDIU	sp,FRAMESZ
	ret

	/* Something bad happened -- no child created */
error:
	PTR_ADDIU	sp,FRAMESZ
#ifdef PIC
	la		t9,__syscall_error
	jr		t9
#else
	j		__syscall_error
#endif
	END(__clone)

/* Load up the arguments to the function.  Put this block of code in
   its own function so that we can terminate the stack trace with our
   debug info.

   At this point we have t0=nargs, t9=fn, sp=&arg[0].  */

NESTED(__thread_start,32,sp)
	/* Stackframe has been created on entry of clone() */
	/* Calculate address of jump into argument loading code */
	li		t1,MAX_REG_ARGS
	slt		t0,t1,t2       /* max MAX_REG_ARGS args in registers */
	MOVN		(t2,t1,t0)
	la		v0,arg0
	PTR_SLL		t1,t0,PTR_SCALESHIFT
	PTR_SUBU	v0,t1
	jr		v0

	/* Load the integer register arguments */
	REG_L		a0,SZREG(sp)
arg0:

	/* Call the user's function */
	jalr		t9

	/* Call _exit rather than doing it inline for breakpoint purposes */
	move		a0,v0
	jal		_exit

	END(__thread_start)

weak_alias(__clone, clone)
