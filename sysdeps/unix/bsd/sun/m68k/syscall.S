/* Copyright (C) 1993, 1997 Free Software Foundation, Inc.
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
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <sysdep.h>

ENTRY (syscall)
	movel sp@, d0		/* Save return address in D0.  */
	movel sp@(4), sp@	/* Put syscall number at top of stack.  */
	movel d0, sp@(4)	/* Put return address under it.  */
	trap #0			/* Do syscall; pops number from stack.  */
	jcs error
	ret
error:	jmp syscall_error
