/* Copyright (C) 1993 Free Software Foundation, Inc.
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
#include <limits.h>

.globl	__tzname_max
ENTRY (sysconf)
	cmpl 4(%esp), $_SC_TZNAME_MAX /* Is the arg _SC_TZNAME_MAX?  */
	je tzname
	DO_CALL (sysconf, 1)	/* No; use the SCO system call.  */
	ret
tzname:	movl (C_SYMBOL_NAME(__tzname_max)), %eax /* Yes; use __tzname_max. */
#ifdef	TZNAME_MAX
	cmpl $TZNAME_MAX, %eax	/* Is TZNAME_MAX larger?  */
	jle out
	movl $TZNAME_MAX, %eax	/* Yes; return it.  */
out:
#endif
	ret
