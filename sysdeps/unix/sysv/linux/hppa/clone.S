/* Copyright (C) 1996, 1997, 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by David Huggins-Daines <dhd@debian.org>, 2000.
   Based on the Alpha version by Richard Henderson <rth@tamu.edu>, 1996.

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

#include <asm/unistd.h>
#include <sysdep.h>
#define _ERRNO_H	1
#include <bits/errno.h>

/* int clone(int (*fn)(void *arg), void *child_stack, int flags, void *arg) */

        .text
ENTRY(__clone)
	/* FIXME: I have no idea how profiling works on hppa. */

	/* Sanity check arguments.  */
	comib,=  0,%arg0,.Lerror        /* no NULL function pointers */
	ldi     -EINVAL,%ret0
	comib,=  0,%arg1,.Lerror        /* no NULL stack pointers */
	nop

	/* Save the fn ptr and arg on the new stack.  */
	stwm    %arg0,64(%arg1)
	stw	%arg3,-60(%arg1)

	/* Save the PIC register. */
#ifdef PIC
	stw	%r19,-32(%sr0, %sp)	/* parent */
#endif

	/* Do the system call */
	copy	%arg2,%arg0
	ble     0x100(%sr2,%r0)
	ldi	__NR_clone,%r20

	ldi	-4096,%r1
	comclr,>>= %r1,%ret0,%r0	/* Note: unsigned compare. */
	b,n	.Lerror

	comib,=,n 0,%ret0,thread_start

	/* Successful return from the parent
	   No need to restore the PIC register, 
	   since we return immediately. */

	bv	%r0(%rp)
	nop

	/* Something bad happened -- no child created */
.Lerror:

	/* Restore the PIC register on error */
#ifdef PIC
	ldw	-32(%sr0, %sp), %r19	/* parent */
#endif

	b	__syscall_error
	sub     %r0,%ret0,%arg0

thread_start:

	/* Load up the arguments.  */
	ldw	-60(%sr0, %sp),%arg0
	ldw     -64(%sr0, %sp),%r22

	/* $$dyncall fixes childs PIC register */

	/* Call the user's function */
	bl	$$dyncall,%r31
	copy	%r31,%rp

	bl	_exit,%rp
	copy	%ret0,%arg0

	/* Die horribly.  */
	iitlbp	%r0,(%r0)

PSEUDO_END(__clone)

weak_alias(__clone, clone)
