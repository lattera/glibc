/* Copyright (C) 1996, 1997, 2002 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Richard Henderson <rth@tamu.edu>, 1996.

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

/* clone() is even more special than fork() as it mucks with stacks
   and invokes a function in the right context after its all over.  */

#include <sysdep.h>
#define _ERRNO_H	1
#include <bits/errno.h>

#define CLONE_VM	0x00000100
#define CLONE_THREAD	0x00010000

/* int clone(int (*fn)(void *arg), void *child_stack, int flags,
	     void *arg, pid_t *ptid, void *tls, pid_t *ctid);

   Note that everything past ARG is technically optional, based
   on FLAGS, and that CTID is arg 7, and thus is on the stack.
   However, since a load from top-of-stack better be legal always,
   we don't bother checking FLAGS.  */

        .text
	.align	4
	.globl	__clone
	.ent	__clone
	.usepv	__clone, USEPV_PROF

	cfi_startproc
__clone:
#ifdef PROF
	ldgp	gp,0(pv)
	lda	AT, _mcount
	jsr	AT, (AT), _mcount
#endif

	/* Sanity check arguments.  */
	ldiq	v0, EINVAL
	beq	a0, SYSCALL_ERROR_LABEL	/* no NULL function pointers */
	beq	a1, SYSCALL_ERROR_LABEL	/* no NULL stack pointers */

	/* Save the fn ptr and arg on the new stack.  */
	subq	a1, 32, a1
	stq	a0, 0(a1)
	stq	a3, 8(a1)
#ifdef RESET_PID
	stq	a2, 16(a1)
#endif

	/* The syscall is of the form clone(flags, usp, ptid, ctid, tls).
	   Shift the flags, ptid, ctid, tls arguments into place; the
	   child_stack argument is already correct.  */
	mov	a2, a0
	mov	a4, a2
	ldq	a3, 0(sp)
	mov	a5, a4

	/* Do the system call.  */
	ldiq	v0, __NR_clone
	call_pal PAL_callsys

	bne	a3, SYSCALL_ERROR_LABEL
	beq	v0, thread_start

	/* Successful return from the parent.  */
	ret

PSEUDO_END(__clone)
	cfi_endproc

/* Load up the arguments to the function.  Put this block of code in
   its own function so that we can terminate the stack trace with our
   debug info.  */

	.ent thread_start
	cfi_startproc
thread_start:
	mov	0, fp
	cfi_def_cfa_register(fp)
	cfi_undefined(ra)

#ifdef RESET_PID
	/* Check and see if we need to reset the PID.  */
	ldq	t0, 16(sp)
	lda	t1, CLONE_THREAD
	and	t0, t1, t2
	beq	t2, 2f
1:
#endif

	/* Load up the arguments.  */
	ldq	pv, 0(sp)
	ldq	a0, 8(sp)
	addq	sp, 32, sp

	/* Call the user's function.  */
	jsr	ra, (pv)
	ldgp	gp, 0(ra)

	/* Call _exit rather than doing it inline for breakpoint purposes.  */
	mov	v0, a0
#ifdef PIC
	bsr	ra, HIDDEN_JUMPTARGET(_exit)	!samegp
#else
	jsr	ra, HIDDEN_JUMPTARGET(_exit)
#endif

	/* Die horribly.  */
	halt

#ifdef RESET_PID
2:
	rduniq
	lda	t1, CLONE_VM
	mov	v0, s0
	lda	v0, -1
	and	t0, t1, t2
	bne	t2, 3f
	lda	v0, __NR_getxpid
	callsys
3:
	stl	v0, PID_OFFSET(s0)
	stl	v0, TID_OFFSET(s0)
	br	1b
#endif
	cfi_endproc
	.end thread_start

weak_alias (__clone, clone)
