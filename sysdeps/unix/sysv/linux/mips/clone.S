/* Copyright (C) 1996, 1997, 2000 Free Software Foundation, Inc.
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

/* int clone(int (*fn)(), void *child_stack, int flags, void *arg) */

#define FRAMESZ  8*SZREG
#if _MIPS_SIM == _MIPS_SIM_ABI32
#define MAX_REG_ARGS 4
#else
#define MAX_REG_ARGS 6
#endif

	.text
NESTED(__clone,FRAMESZ,sp)
#ifdef __PIC__
	.set		noreorder
	.cpload		$25
	.set		reorder
	PTR_SUBIU	sp,FRAMESZ
	.cprestore	SZREG*4
#else
	PTR_SUBIU	sp,FRAMESZ
#endif
#ifdef PROF
	.set		noat
	move		$1,ra
	jal		_mcount
	.set		at
#endif

	REG_S		s0,FRAMESZ-SZREG*2(sp)
	REG_S		s1,FRAMESZ-SZREG*3(sp)
	/* Sanity check arguments.  */
	li		v0,EINVAL
	beqz		a0,error	/* no NULL function pointers */
	beqz		a1,error	/* no NULL stack pointers */

	/* Allocate space on the new stack and copy args over */
	/* Save the arg for user's function */
	move		s0,a3		/* Save arg __thread_start.  */
	move		s1,a0		/* Save func. pointer.  */


	/* Do the system call */
	move		a0,a2
	li		v0,__NR_clone
	syscall

	bnez		a3,error
	beqz		v0,__thread_start

	/* Successful return from the parent */
	REG_L		s0,FRAMESZ-SZREG*2(sp)
	REG_L		s1,FRAMESZ-SZREG*3(sp)
	PTR_ADDIU	sp,FRAMESZ
	ret

	/* Something bad happened -- no child created */
error:
	REG_L		s0,FRAMESZ-SZREG*2(sp)
	REG_L		s1,FRAMESZ-SZREG*3(sp)
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

   At this point we have s0=arg, s1=fn.  */

NESTED(__thread_start,FRAMESZ,sp)
	/* The stackframe has been created on entry of clone().  */
	/* Resort the arg for user's function.  */
	move		a0,s0
	move		t9,s1

	/* Call the user's function.  */
	jalr		t9

	/* Call _exit rather than doing it inline for breakpoint purposes.  */
	move		a0,v0
#ifdef __PIC__
	la		t9,_exit
	jalr		t9
#else
	jal		_exit
#endif
	END(__thread_start)

weak_alias(__clone, clone)
