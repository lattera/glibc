/* Copyright (C) 2000, 2001 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <sysdep.h>

/* Please consult the file sysdeps/unix/sysv/linux/s390/sysdep.h for
   more information about the value -4095 used below.*/

ENTRY (syscall)
	/* Save registers and setup stack.  */
	stm     %r6,%r15,24(%r15)  /* save registers */
	lr      %r1,%r15
	l       %r0,4(0,%r15)      /* load eos */
	ahi     %r15,-96           /* buy stack space */
	st      %r1,0(0,%r15)      /* store back chain */
	st      %r0,4(0,%r15)      /* store eos */

	lr     %r1,%r2             /* move syscall number */
	lr     %r2,%r3             /* first parameter  */
	lr     %r3,%r4             /* second parameter */
	lr     %r4,%r5             /* third parameter  */
	lr     %r5,%r6             /* fourth parameter */
	l      %r6,192(%r15)       /* fifth parameter  */
	basr   %r7,0
.L0:    ex     %r1,.L1-.L0(%r7)    /* lsb of R1 is subsituted as SVC number */

	l       %r15,0(0,%r15)     /* load back chain */
	lm      %r6,15,24(%r15)    /* load registers */

	lhi    %r0,-4095
	clr    %r2,%r0             /* check R2 for error */
	jnl    SYSCALL_ERROR_LABEL
	br     %r14                /* return to caller */
.L1:    .word  0x0A00              /* opcode for SVC 0 */
PSEUDO_END (syscall)
