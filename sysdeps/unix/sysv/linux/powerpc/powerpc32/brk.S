/* brk system call for Linux/ppc.
   Copyright (C) 1995-97, 1999, 2000, 2006, 2011 Free Software Foundation, Inc.
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
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA
   02110-1301 USA.  */

#include <sysdep.h>
#define _ERRNO_H	1
#include <bits/errno.h>
#include <bp-sym.h>
#include <bp-asm.h>

	.comm	__curbrk,4,4
	.section ".text"
ENTRY (BP_SYM (__brk))
	DISCARD_BOUNDS (r3)	/* the bounds are meaningless, so toss 'em */
	mflr	r0
	stwu    r1,-16(r1)
	cfi_adjust_cfa_offset (16)
	stw	r3,8(r1)
	stw	r0,20(r1)
	cfi_offset (lr, 4)
	DO_CALL(SYS_ify(brk))
	lwz     r6,8(r1)
#ifdef PIC
	SETUP_GOT_ACCESS(r5,got_label)
	addis	r5,r5,__curbrk-got_label@ha
	stw	r3,__curbrk-got_label@l(r5)
#else
	lis     r4,__curbrk@ha
	stw     r3,__curbrk@l(r4)
#endif
	lwz	r0,20(r1)
	cmplw   r6,r3
	addi    r1,r1,16
	mtlr	r0
	li	r3,0
	blelr+
	li      r3,ENOMEM
	b	__syscall_error@local
END (BP_SYM (__brk))

weak_alias (BP_SYM (__brk), BP_SYM (brk))
