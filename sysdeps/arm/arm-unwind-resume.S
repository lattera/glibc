/* _Unwind_Resume wrapper for ARM EABI.
   Copyright (C) 2015-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include <sysdep.h>

/* This is just implementing exactly what the C version does.
   We do it in assembly just to ensure that we get an unmolested tail
   call to the libgcc function, which is necessary for the ARM unwinder.  */

ENTRY (_Unwind_Resume)
	LDR_HIDDEN (ip, ip, __libgcc_s_resume, 0)
	cmp	ip, #0
	beq	1f
0:	PTR_DEMANGLE (ip, ip, r2, r3)
	bx	ip

	/* We need to save and restore LR (for our own return address)
	   and R0 (for the argument to _Unwind_Resume) around the call.  */
1:	push	{r0, lr}
	cfi_adjust_cfa_offset (8)
	cfi_rel_offset (r0, 0)
	cfi_rel_offset (lr, 4)
	bl	__libgcc_s_init
	pop	{r0, lr}
	cfi_adjust_cfa_offset (-8)
	cfi_restore (r0)
	cfi_restore (lr)

	LDR_HIDDEN (ip, ip, __libgcc_s_resume, 0)
	b	0b
END (_Unwind_Resume)
