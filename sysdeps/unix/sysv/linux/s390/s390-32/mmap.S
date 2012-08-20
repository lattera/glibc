/* Copyright (C) 2000-2012 Free Software Foundation, Inc.
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
#include <kernel-features.h>

#define EINVAL	22

	.text
ENTRY(__mmap)
	/* Save registers and setup stack frame.  */
	stm     %r6,%r15,24(%r15)
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
	ahi     %r15,-120               /* buy stack space */
	cfi_adjust_cfa_offset (120)
	st      %r1,0(%r15)             /* store back chain */

	/* Store parameters on stack, because mmap2
	 * takes only one parameter: a pointer to the parameter area
	 */
	st      %r6,0x70(%r15)          /* Store 'fd'.      */
	st      %r5,0x6C(%r15)          /* Store 'flags'.   */
	st      %r4,0x68(%r15)          /* Store 'prot'.    */
	st      %r3,0x64(%r15)          /* Store 'length'.  */
	st      %r2,0x60(%r15)          /* Store 'start'.   */
	l       %r1,216(%r15)           /* Load offset.     */

	tml     %r1,0x0fff              /* Offset page aligned ?  */
	lhi     %r2,-EINVAL
	jnz     1f                      /* No -> EINVAL.  */
	srl     %r1,12                  /* mmap2 takes the offset in pages.  */
	st      %r1,0x74(%r15)          /* Store page offset.  */
	la      %r2,0x60(%r15)          /* Load address of parameter list.  */
	svc     SYS_ify(mmap2)          /* Do the system call trap.  */

1:	l       %r15,0(%r15)            /* Load back chain.  */
	cfi_adjust_cfa_offset (-120)
	lm      %r6,%r15,24(%r15)       /* Load registers.  */

	/* check gpr 2 for error */
	lhi     %r0,-4096
	clr     %r2,%r0
	jnl     SYSCALL_ERROR_LABEL

	/* Successful; return the syscall's value.  */
	br      %r14

PSEUDO_END (__mmap)

weak_alias (__mmap, mmap)
