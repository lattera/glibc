/* Copyright (C) 1993, 1995, 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Brendan Kehoe (brendan@zen.org).

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

#ifndef SYS_brk
#define SYS_brk 17
#endif

#ifndef       HAVE_GNU_LD
#define __end           end
#endif

.data
	.extern __end,8
	.globl __curbrk
__curbrk:
	.quad __end

.text
ENTRY(__brk)
	/* FIXME We do not check for asking for less than a page yet. */
	ldiq v0, SYS_brk
	call_pal PAL_callsys
	bne a3, error

	/* Update __curbrk and exit cleanly.  */
/*	ldgp gp, 0(t12) */
	stq a0, __curbrk

	mov zero, v0
	ret
	/* What a horrible way to die.  */
error:	ldgp gp,0(gp)
	jmp zero,syscall_error
	.end __brk

weak_alias (__brk, brk)
