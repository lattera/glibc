/* Copyright (C) 2000, 2003, 2005, 2009 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include <sysdep.h>

#define	EINVAL		22

#ifdef __ARMEB__
# define LOW_OFFSET      8 + 4
/* The initial + 4 is for the stack postdecrement.  */
# define HIGH_OFFSET 4 + 8 + 0
#else
# define LOW_OFFSET      8 + 0
# define HIGH_OFFSET 4 + 8 + 4
#endif

	/* The mmap2 system call takes six arguments, all in registers.  */
	.text
ENTRY (__mmap64)
	ldr	ip, [sp, $LOW_OFFSET]
	str	r5, [sp, #-4]!
	cfi_adjust_cfa_offset (4)
	cfi_rel_offset (r5, 0)
	ldr	r5, [sp, $HIGH_OFFSET]
	str	r4, [sp, #-4]!
	cfi_adjust_cfa_offset (4)
	cfi_rel_offset (r4, 0)
	cfi_remember_state
	movs	r4, ip, lsl $20		@ check that offset is page-aligned
	mov	ip, ip, lsr $12
	moveqs	r4, r5, lsr $12		@ check for overflow
	bne	.Linval
	ldr	r4, [sp, $8]		@ load fd
	orr	r5, ip, r5, lsl $20	@ compose page offset
	DO_CALL (mmap2, 0)
	cmn	r0, $4096
	ldmfd	sp!, {r4, r5}
	cfi_adjust_cfa_offset (-8)
	cfi_restore (r4)
	cfi_restore (r5)
	RETINSTR(cc, lr)
	b	PLTJMP(syscall_error)

	cfi_restore_state
.Linval:
	mov	r0, $-EINVAL
	ldmfd	sp!, {r4, r5}
	cfi_adjust_cfa_offset (-8)
	cfi_restore (r4)
	cfi_restore (r5)
	b	PLTJMP(syscall_error)
PSEUDO_END (__mmap64)

weak_alias (__mmap64, mmap64)
