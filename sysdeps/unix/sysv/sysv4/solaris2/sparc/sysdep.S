/* Copyright (C) 1993 Free Software Foundation, Inc.
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
#include <errnos.h>

ENTRY(syscall_error)
	/* If it was a syscall that got interrupted, but can
	   be restarted, drop ERESTART in.  */
	cmp %o0, ERESTART
	be,a notint
	mov EINTR, %o0

	/* Store it in errno... */
notint:	sethi %hi(C_SYMBOL_NAME(errno)), %g1
	st %o0, [%g1 + %lo(C_SYMBOL_NAME(errno))]

	/* And just kick back a -1.  */
	retl
	mov -1, %o0
