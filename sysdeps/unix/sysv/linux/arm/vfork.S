/* Copyright (C) 1999 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Philip Blundell <philb@gnu.org>

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <sysdep.h>
#define _ERRNO_H	1
#include <bits/errno.h>
#define _SIGNAL_H
#include <bits/signum.h>

#define CLONE_VM      0x00000100 /* Set if VM shared between processes.  */
#define CLONE_VFORK   0x00004000 /* Set if the parent wants the child to
				     wake it up on mm_release.  */
	
/* Clone the calling process, but without copying the whole address space.
   The calling process is suspended until the new process exits or is
   replaced by a call to `execve'.  Return -1 for errors, 0 to the new process,
   and the process ID of the new process to the old process.  */

ENTRY (__vfork)
	mov	a1, $SIGCLD
	orr	a1, a1, $(CLONE_VM | CLONE_VFORK)
	mov	a2, $0
	swi	SYS_ify(clone)
	cmn	a1, $4096
	bhs	PLTJMP(C_SYMBOL_NAME(__syscall_error))
	RETINSTR(mov, pc, lr)
	
PSEUDO_END (__vfork)

weak_alias (__vfork, vfork)
