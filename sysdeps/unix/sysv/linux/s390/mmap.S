/* Copyright (C) 2000 Free Software Foundation, Inc.
   Contributed by Martin Schwidefsky (schwidefsky@de.ibm.com).
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

	.text

ENTRY(__mmap)
	/* Save registers and setup stack frame.  */
	stm     %r6,%r15,24(%r15)
	lr      %r1,%r15
	l       %r0,4(0,%r15)           /* load eos */
	ahi     %r15,-120               /* buy stack space */
	st      %r1,0(0,%r15)           /* store back chain */
	st      %r0,4(0,%r15)           /* store eos */

	/* Store parameters on stack, because old_mmap
	 * takes only one parameter: a pointer to the parameter area
	 */
	mvc     0x74(4,%r15),216(%r15)  /* move 'offset'  */
	st      %r6,0x70(0,%r15)        /* store 'fd'     */
	st      %r5,0x6C(0,%r15)        /* store 'flags'  */
	st      %r4,0x68(0,%r15)        /* store 'prot'   */
	st      %r3,0x64(0,%r15)        /* store 'length' */
	st      %r2,0x60(0,%r15)        /* store 'start'  */

	la      %r2,0x60(0,%r15)        /* load address of parameter list */
	/* Do the system call trap.  */
	svc     SYS_ify(mmap)

	l       %r15,0(0,%r15)          /* load back chain */
	lm      %r6,%r15,24(%r15)       /* load registers */

	/* check gpr 2 for error */
	lhi     %r0,-4096
	clr     %r2,%r0
	jnl     SYSCALL_ERROR_LABEL

	/* Successful; return the syscall's value.  */
	br      %r14

PSEUDO_END (__mmap)

weak_alias (__mmap, mmap)
