/* Copyright (C) 1996 Free Software Foundation, Inc.
   Contributed by Richard Henderson (rth@tamu.edu)

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

/* clone() is even more special than fork() as it mucks with stacks
   and invokes a function in the right context after its all over.  */

#include <sysdep.h>
#include <errnos.h>

/* int clone(int (*fn)(), void *child_stack, int flags, int nargs, ...) */

        .text
ENTRY(__clone)
	lda	sp,-16(sp)
	.frame	sp,16,$26,0
	/* Save rest of argument registers for varargs-type work.  */
	stq	a4,0(sp)
	stq	a5,8(sp)
	.prologue 1

	/* Sanity check arguments.  */
	sextl	a3,a3
	ldiq	v0,EINVAL
	beq	a0,$error		/* no NULL function pointers */
	beq	a1,$error		/* no NULL stack pointers */
	blt	a3,$error		/* no negative argument counts */

	/* Allocate space on the new stack and copy args over */
	mov	a3,t0			/* save nargs for thread_start */
	s8addq	a3,sp,t1
1:	ldq	t2,-8(t1)
	subq	t1,8,t1
	stq	t2,-8(a1)
	subq	a3,1,a3
	subq	a1,8,a1
	bne	a3,1b

	/* Do the system call */
	mov	a0,pv			/* get fn ptr out of the way */
	mov	a2,a0
	ldiq	v0,__NR_clone
	call_pal PAL_callsys

	bne	a3,$error
	beq	v0,thread_start

	/* Successful return from the parent */
	lda	sp,16(sp)
	ret

	/* Something bad happened -- no child created */
$error:
	br	gp,1f
1:	ldgp	gp,0(gp)
	lda	sp,16(sp)
	jmp	zero,__syscall_error

	END(__clone)

/* Load up the arguments to the function.  Put this block of code in
   its own function so that we can terminate the stack trace with our
   debug info.

   At this point we have $t0=nargs, $pv=fn, $sp=&arg[0].  */

	.ent thread_start
thread_start:
	.frame fp,0,zero,0
	mov	zero,fp
	.prologue 0

	/* Calculate address of jump into argument loading code */
	cmple	t0,6,t2		/* no more than 6 args in registers */
	cmoveq	t2,6,t0
	br	v0,1f		/* find address of arg0 */
1:	lda	v0,$arg0-1b(v0)
	s4addq	t0,zero,t1
	subq	v0,t1,v0
	jmp	(v0)

	/* Load the integer register arguments */
	ldq	a5,40(sp)
	ldq	a4,32(sp)
	ldq	a3,24(sp)
	ldq	a2,16(sp)
	ldq	a1,8(sp)
	ldq	a0,0(sp)
$arg0:

	/* Adjust stack to remove the arguments we just loaded */
	s8addq	t0,sp,sp

	/* Call the user's function */
	jsr	ra,(pv)
	ldgp	gp,0(ra)

	/* Call _exit rather than doing it inline for breakpoint purposes */
	mov	v0,a0
	jsr	ra,_exit

	.end thread_start

weak_alias(__clone, clone)
