/* Copyright (C) 1999,2000 Free Software Foundation, Inc.

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

/* clone is even more special than fork as it mucks with stacks
   and invokes a function in the right context after its all over.  */

#include <sysdep.h>

/* int clone(int (*fn)(void *arg), void *child_stack, int flags, void *arg); */

/*
 * Used for both clone and fork system calls.  Note that if fn is NULL
 *   then clone was called from fork.
 */

ENTRY(__clone)
	add	r2 = -16, r33		// save space for fn and arg
	cmp.ne	p6,p0 = 0,r33
	;;
(p6)	add	r33 = -16, r33
	add	r12 = -16, r12
	;;
(p6)	st8	[r2] = r32, 8		// save fn for child
	st8	[r12] = r32		// save fn for parent
	;;
(p6)	st8	[r2] = r35		// save arg for child
	mov	r32 = r34		// put flags in out0
	DO_CALL (SYS_ify (clone))
1:
	ld8	r33 = [r12], 8		// recover fn
	;;
	ld8	r32 = [r12], 8		// recover arg

	cmp.eq p6,p0=-1,r10
(p6)	br.cond.spnt.many __syscall_error // handle error

	cmp.ne p7,p0=0,r8
(p7)	ret				// parent process, just return

	cmp.eq p8,p0=0,r33
(p8)	ret				// fork call, return

	alloc	loc0 = ar.pfs,2,2,2,0
	;;
	ld8	r2 = [in1], 8		// get address of fn
	mov	out0 = in0
	mov	loc1 = gp		// save gp across indirect call
	;;
	ld8	gp = [in1]		// set new gp
	mov	b6 = r2
	br.call.sptk b0 = b6		// call thread routine
	;;

	mov	ar.pfs = loc0
	mov	gp = loc1
	mov	r32 = r8		// exit value
	br.call.sptk	b0 = _exit	// we're out of here

PSEUDO_END(__clone)

weak_alias (__clone, clone)
