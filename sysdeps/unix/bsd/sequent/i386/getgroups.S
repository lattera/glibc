/* Copyright (C) 1994, 1995, 1997 Free Software Foundation, Inc.
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
#include <limits.h>

/* Dynix erroneously reports `getgroups (0, 0)' as an error.
   We fix up for that case.  */

#define syscall_error myerror
SYSCALL__ (getgroups, 2)
#undef syscall_error
	ret
myerror:
	tstl 4(%esp)		/* Was the first arg 0?  */
	jnz syscall_error	/* If not, go to the normal error case.  */
	/* When called with (0, 0), we want to return the number of groups
	   without storing anything.  The Dynix system call gives an error
	   for this case, so we fix up by calling it with a local array we
	   never use, and just use the return value.  */
	subl %esp, $(NGROUPS_MAX * 4) /* Allocate a local array.  */
	movl $NGROUPS_MAX, %ecx	/* Pass NGROUPS_MAX for first arg.  */
	movl %esp, %edx		/* Pass local array for second arg.  */
	DO_CALL (getgroups, 2)	/* Do the system call.  */
	addl %esp, $(NGROUPS_MAX * 4) /* Pop the local array.  */
	jb syscall_error	/* Check for error from the system call.  */
	ret			/* Return its value.  */

weak_alias (__getgroups, getgroups)
