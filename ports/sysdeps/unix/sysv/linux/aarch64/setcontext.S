/* Set current context.

   Copyright (C) 2009-2012 Free Software Foundation, Inc.

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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <sysdep.h>
#include "ucontext_i.h"
#include "ucontext-internal.h"

/* int setcontext (const ucontext_t *ucp) */

	.text

ENTRY(__setcontext)

	/* Create a signal frame on the stack:

		fp
		lr
		...
	   sp-> rt_sigframe
	 */

	stp     x29, x30, [sp, -16]!
	cfi_adjust_cfa_offset (16)
	cfi_rel_offset (x29, 0)
	cfi_rel_offset (x30, 8)

        mov     x29, sp
	cfi_def_cfa_register (x29)

	/* Allocate space for the sigcontext.  */
	mov	w3, #((RT_SIGFRAME_SIZE + SP_ALIGN_SIZE) & SP_ALIGN_MASK)
	sub	sp, sp,	x3

	/* Compute the base address of the ucontext structure.  */
	add	x1, sp, #RT_SIGFRAME_UCONTEXT

	/* Only ucontext is required in the frame, *copy* it in.  */

#if UCONTEXT_SIZE % 16
#error The implementation of setcontext.S assumes sizeof(ucontext_t) % 16 == 0
#endif

	mov	x2, #UCONTEXT_SIZE / 16
0:
	ldp	x3, x4, [x0], #16
	stp	x3, x4, [x1], #16
	sub	x2, x2, 1
	cbnz	x2, 0b

	/* rt_sigreturn () -- no arguments, sp points to struct rt_sigframe.  */
	mov	x8, SYS_ify (rt_sigreturn)
	svc	0

	/* Ooops we failed.  Recover the stack */

	mov	sp, x29
	cfi_def_cfa_register (sp)

        ldp     x29, x30, [sp], 16
	cfi_adjust_cfa_offset (16)
	cfi_restore (x29)
	cfi_restore (x30)
	b	C_SYMBOL_NAME(__syscall_error)

PSEUDO_END (__setcontext)
weak_alias (__setcontext, setcontext)

ENTRY(__startcontext)
	mov	x0, x19
	cbnz	x0, __setcontext
1:	b       HIDDEN_JUMPTARGET(_exit)
END(__startcontext)
