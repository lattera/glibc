/* Copyright (C) 1993, 1994 Free Software Foundation, Inc.
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
#include <confname.h>

.globl	__tzname_max
ENTRY (__sysconf)
	cmpl $_SC_TZNAME_MAX, 4(%esp) /* Is the arg _SC_TZNAME_MAX?  */
	je tzname
	DO_CALL (sysconf, 1)	/* No; use the SCO system call.  */
	ret
tzname:	jmp C_SYMBOL_NAME(__tzname_max) /* Yes; bounce to __tzname_max (). */
