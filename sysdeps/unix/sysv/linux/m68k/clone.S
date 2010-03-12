/* Copyright (C) 1996,97,98,2002,2010 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Andreas Schwab (schwab@issan.informatik.uni-dortmund.de)

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

/* clone() is even more special than fork() as it mucks with stacks
   and invokes a function in the right context after its all over.  */

#include <sysdep.h>
#define _ERRNO_H	1
#include <bits/errno.h>
#ifdef RESET_PID
#include <tls.h>
#endif

#define CLONE_VM      0x00000100
#define CLONE_THREAD  0x00010000

/* int clone(int (*fn)(void *arg), void *child_stack, int flags, void *arg,
	     void *parent_tidptr, void *tls, void *child_tidptr) */

        .text
ENTRY (__clone)

	/* Sanity check arguments.  */
	movel	#-EINVAL, %d0
	movel	4(%sp), %a0		/* no NULL function pointers */
	tstl	%a0
	jeq	SYSCALL_ERROR_LABEL
	movel	8(%sp), %a1		/* no NULL stack pointers */
	tstl	%a1
	jeq	SYSCALL_ERROR_LABEL

	/* Allocate space and copy the argument onto the new stack.  */
	movel	16(%sp), -(%a1)

	/* Do the system call */
	movel	12+0(%sp), %d1		/* get flags */
	movel	%d3, -(%a1)             /* save %d3 and get parent_tidptr */
	movel	%d3, -(%sp)
	movel	20+4(%sp), %d3
	movel	%d4, -(%a1)		/* save %d4 and get child_tidptr */
	movel	%d4, -(%sp)
	movel	28+8(%sp), %d4
	movel	%d5, -(%a1)             /* save %d5 and get tls */
	movel	%d5, -(%sp)
	movel	24+12(%sp), %d5
	/* save %d2 and get stack pointer */
#ifdef __mcoldfire__
	movel	%d2, -(%a1)
	movel	%d2, -(%sp)
	movel	%a1, %d2
#else
	exg	%d2, %a1		/* save %d2 and get stack pointer */
#endif
	movel	#SYS_ify (clone), %d0
	trap	#0
#ifdef __mcoldfire__
	movel	(%sp)+, %d2
#else
	exg	%d2, %a1		/* restore %d2 */
#endif
	movel	(%sp)+, %d5             /* restore %d5, %d4 and %d3 */
	movel	(%sp)+, %d4
	movel	(%sp)+, %d3

	tstl	%d0
	jmi	SYSCALL_ERROR_LABEL
	jeq	thread_start

	rts

thread_start:
	cfi_startproc
	cfi_undefined (pc)	/* Mark end of stack */
	subl	%fp, %fp	/* terminate the stack frame */
#ifdef RESET_PID
	/* Check and see if we need to reset the PID.  */
	movel	%d1, %a1
	andl	#CLONE_THREAD, %d1
	jne	donepid
	movel	%a1, %d1
	movel	#-1, %d0
	andl	#CLONE_VM, %d1
	jne	gotpid
	movel	#SYS_ify (getpid), %d0
	trap	#0
gotpid:
	movel	%a0, -(%sp)
	movel	%d0, -(%sp)
	bsrl	__m68k_read_tp@PLTPC
	movel	(%sp)+, %d0
	movel	%d0, PID_OFFSET(%a0)
	movel	%d0, TID_OFFSET(%a0)
	movel	(%sp)+, %a0
donepid:
#endif
	jsr	(%a0)
	movel	%d0, %d1
	movel	#SYS_ify (exit), %d0
	trap	#0
	cfi_endproc

PSEUDO_END (__clone)

weak_alias (__clone, clone)
