/* Copyright (C) 1996 Free Software Foundation, Inc.
This file is part of the GNU C Library.

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

	.text
	SYSCALL_ERROR_HANDLER
ENTRY (syscall)
	move.l (%sp)+, %a0	/* Pop return address.  */
	DO_CALL ((%sp), 5)	/* Frob the args and do the system call.  */
	tst.l %d0		/* Check %d0 for error.  */
	jmi error		/* Jump to error handler if negative.  */
	jmp (%a0)		/* Return to caller.  */

error:	pea (%a0)
	jra syscall_error
