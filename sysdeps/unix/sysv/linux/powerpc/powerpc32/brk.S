/* brk system call for Linux/ppc.
   Copyright (C) 1995-2016 Free Software Foundation, Inc.
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
#define _ERRNO_H	1
#include <bits/errno.h>

	.comm	__curbrk,4,4
	.section ".text"
ENTRY (__brk)
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
END (__brk)

weak_alias (__brk, brk)
