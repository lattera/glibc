/* Copyright (C) 2001 Free Software Foundation, Inc.
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

/* Please consult the file sysdeps/unix/sysv/linux/s390/sysdep.h for
   more information about the value -4095 used below.*/

ENTRY (syscall)
	/* Save registers and setup stack.  */
	stmg	%r6,%r15,48(%r15)  /* Save registers.  */
	lgr	%r1,%r15
	lg	%r0,8(%r15)	   /* Load eos.	 */
	aghi	%r15,-160	   /* Buy stack space.	*/
	stg	%r1,0(%r15)	   /* Store back chain.	 */
	stg	%r0,8(%r15)	   /* Store eos.  */
	
	lgr    %r1,%r2		   /* Move syscall number.  */
	lgr    %r2,%r3		   /* First parameter.	*/
	lgr    %r3,%r4		   /* Second parameter.	 */
	lgr    %r4,%r5		   /* Third parameter.	*/
	lgr    %r5,%r6		   /* Fourth parameter.	 */
	lg     %r6,320(%r15)	   /* Fifth parameter.	*/
	basr   %r7,0
.L0:	ex     %r1,.L1-.L0(%r7)	   /* Lsb of R1 is subsituted as SVC number.  */

	lg	%r15,0(%r15)	   /* Load back chain.	*/
	lmg	%r6,15,48(%r15)	   /* Load registers.  */

	lghi   %r0,-4095
	clgr   %r2,%r0		   /* Check R2 for error.  */
	jnl    SYSCALL_ERROR_LABEL
	br     %r14		   /* Return to caller.	 */
.L1:	.word  0x0A00		   /* Opcode for SVC 0.	 */
PSEUDO_END (syscall)

