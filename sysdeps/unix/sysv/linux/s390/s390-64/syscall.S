/* Copyright (C) 2001-2018 Free Software Foundation, Inc.
   Contributed by Martin Schwidefsky (schwidefsky@de.ibm.com).
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <sysdep.h>

/* Please consult the file sysdeps/unix/sysv/linux/s390/sysdep.h for
   more information about the value -4095 used below.*/

ENTRY (syscall)
	/* Save registers and setup stack.  */
	stmg	%r6,%r15,48(%r15)  /* Save registers.  */
	cfi_offset (%r15,-40)
	cfi_offset (%r14,-48)
	cfi_offset (%r13,-56)
	cfi_offset (%r12,-64)
	cfi_offset (%r11,-72)
	cfi_offset (%r10,-80)
	cfi_offset (%r9,-88)
	cfi_offset (%r8,-96)
	cfi_offset (%r7,-104)
	cfi_offset (%r6,-112)
	lgr	%r1,%r15
	lg	%r0,8(%r15)	   /* Load eos.	 */
	aghi	%r15,-160	   /* Buy stack space.	*/
	cfi_adjust_cfa_offset (160)
	stg	%r1,0(%r15)	   /* Store back chain.	 */
	stg	%r0,8(%r15)	   /* Store eos.  */

	lgr    %r1,%r2		   /* Move syscall number.  */
	lgr    %r2,%r3		   /* First parameter.	*/
	lgr    %r3,%r4		   /* Second parameter.	 */
	lgr    %r4,%r5		   /* Third parameter.	*/
	lgr    %r5,%r6		   /* Fourth parameter.	 */
	lg     %r6,320(%r15)	   /* Fifth parameter.	*/
	lg     %r7,328(%r15)	   /* Sixth parameter.	*/

	svc    0
	lg     %r15,0(%r15)        /* Load back chain.  */
	cfi_adjust_cfa_offset (-160)
	lmg    %r6,%r15,48(%r15)   /* Load registers.  */

	lghi   %r0,-4095
	clgr   %r2,%r0		   /* Check R2 for error.  */
	jgnl   SYSCALL_ERROR_LABEL
	br     %r14		   /* Return to caller.	 */
PSEUDO_END (syscall)

