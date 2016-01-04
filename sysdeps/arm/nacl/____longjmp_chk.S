/* longjmp that validates stack addresses.  ARM/NaCl version.
   Copyright (C) 2015-2016 Free Software Foundation, Inc.
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

	.section .rodata.str1.1,"aMS",%progbits,1
	.type	longjmp_msg,%object
longjmp_msg:
	.string "longjmp causes uninitialized stack frame"
	.size	longjmp_msg, .-longjmp_msg

	.text

/* We don't have sigaltstack and so any longjmp must be to an outer frame.  */
.macro check_sp reg
	cmp sp, \reg
	bls .Lok
#ifdef PIC
	movw r0, #:lower16:longjmp_msg-(.LPIC0+8)
	movt r0, #:upper16:longjmp_msg-(.LPIC0+8)
.LPIC0:	add r0, r0, pc
#else
	movw r0, #:lower16:longjmp_msg
	movt r0, #:upper16:longjmp_msg
#endif
	b HIDDEN_JUMPTARGET(__fortify_fail)
.Lok:
.endm

#define CHECK_SP(reg)	check_sp reg
#define __longjmp	____longjmp_chk
#include <__longjmp.S>
