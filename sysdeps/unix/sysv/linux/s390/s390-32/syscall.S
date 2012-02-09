/* Copyright (C) 2000, 2001, 2006 Free Software Foundation, Inc.
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
	stm     %r6,%r15,24(%r15)  /* save registers */
	cfi_offset (%r15, -36)
	cfi_offset (%r14, -40)
	cfi_offset (%r13, -44)
	cfi_offset (%r12, -48)
	cfi_offset (%r11, -52)
	cfi_offset (%r10, -56)
	cfi_offset (%r9, -60)
	cfi_offset (%r8, -64)
	cfi_offset (%r7, -68)
	cfi_offset (%r6, -72)
	lr      %r1,%r15
	l       %r0,4(0,%r15)      /* load eos */
	ahi     %r15,-96           /* buy stack space */
	cfi_adjust_cfa_offset (96)
	st      %r1,0(0,%r15)      /* store back chain */
	st      %r0,4(0,%r15)      /* store eos */

	lr     %r1,%r2             /* move syscall number */
	lr     %r2,%r3             /* first parameter  */
	lr     %r3,%r4             /* second parameter */
	lr     %r4,%r5             /* third parameter  */
	lr     %r5,%r6             /* fourth parameter */
	l      %r6,192(%r15)       /* fifth parameter  */
	l      %r7,196(%r15)       /* sixth parameter  */

	basr   %r8,0
0:	cl     %r1,4f-0b(%r8)      /* svc number < 256? */
	jl     2f
1:	svc    0
	j      3f
2:	ex     %r1,1b-0b(%r8)      /* lsb of R1 is subsituted as SVC number */
3:	l      %r15,0(%r15)        /* load back chain */
	cfi_adjust_cfa_offset (-96)
	lm     %r6,15,24(%r15)     /* load registers */

	lhi    %r0,-4095
	clr    %r2,%r0             /* check R2 for error */
	jnl    SYSCALL_ERROR_LABEL
	br     %r14                /* return to caller */
4:	.long  256
PSEUDO_END (syscall)
