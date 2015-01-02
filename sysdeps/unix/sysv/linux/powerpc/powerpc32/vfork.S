/* Copyright (C) 2003-2015 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <sysdep.h>
#define _ERRNO_H	1
#include <bits/errno.h>
#include <tcb-offsets.h>

/* Clone the calling process, but without copying the whole address space.
   The calling process is suspended until the new process exits or is
   replaced by a call to `execve'.  Return -1 for errors, 0 to the new process,
   and the process ID of the new process to the old process.  */

ENTRY (__vfork)

	/* Load the TCB-cached PID value and negates it. If It it is zero
	   sets it to 0x800000.  And then sets its value again on TCB field.
	   See raise.c for the logic that relies on this value.  */

	lwz	r0,PID(r2)
	cmpwi	cr0,r0,0
	neg	r0,r0
	bne-	cr0,1f
	lis	r0,0x8000
1:	stw	r0,PID(r2)

	DO_CALL (SYS_ify (vfork))

	cmpwi	cr1,r3,0
	beqlr-	1

	/* Restore the original value of the TCB cache of the PID, if we're
	   the parent.  But in the child (syscall return value equals zero),
	   leave things as they are.  */
	lwz	r0,PID(r2)
	/* Cannot use clrlwi. here, because cr0 needs to be preserved
	   until PSEUDO_RET.  */
	clrlwi	r4,r0,1
	cmpwi	cr1,r4,0
	beq-	cr1,1f
	neg	r4,r0
1:	stw	r4,PID(r2)

	PSEUDO_RET

PSEUDO_END (__vfork)
libc_hidden_def (__vfork)

weak_alias (__vfork, vfork)
strong_alias (__vfork, __libc_vfork)
