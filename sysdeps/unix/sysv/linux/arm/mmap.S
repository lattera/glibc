/* Copyright (C) 1998-2012 Free Software Foundation, Inc.
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

	.text

ENTRY (__mmap)
	/* shuffle args */
	str	r5, [sp, #-4]!
	cfi_adjust_cfa_offset (4)
	cfi_rel_offset (r5, 0)
	ldr	r5, [sp, #8]
	str	r4, [sp, #-4]!
	cfi_adjust_cfa_offset (4)
	cfi_rel_offset (r4, 0)
	cfi_remember_state
	ldr	r4, [sp, #8]

	/* convert offset to pages */
	movs	ip, r5, lsl #20
	bne	.Linval
	mov	r5, r5, lsr #12
	
	/* do the syscall */
	DO_CALL (mmap2, 0)

	/* restore registers */
2:
	ldr	r4, [sp], #4
	cfi_adjust_cfa_offset (-4)
	cfi_restore (r4)
	ldr	r5, [sp], #4
	cfi_adjust_cfa_offset (-4)
	cfi_restore (r5)

	cmn	r0, $4096
	RETINSTR(cc, lr)
	b	PLTJMP(syscall_error)

	cfi_restore_state
.Linval:
	mov	r0, #-EINVAL
	b	2b
PSEUDO_END (__mmap)

weak_alias (__mmap, mmap)
