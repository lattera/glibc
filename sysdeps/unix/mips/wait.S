/* Copyright (C) 1992, 1994 Free Software Foundation, Inc.
   Contributed by Brendan Kehoe (brendan@zen.org).

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <sysdep.h>

.set noreorder

ENTRY(__wait)
	/* Prep it for wait.  */
	move a1, zero
	move a2, zero

	li v0, SYS_wait
	syscall
	beq a3, zero, noerror
	nop
	j syscall_error
	nop

noerror:
	/* If the arg is not NULL, store v1 there.  */
	beq a0, zero, noarg
	nop
	sw v1, 0(a0)
	nop
noarg:
	ret
