/* Copyright (C) 1996 Free Software Foundation, Inc.
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

/* Because the Linux version is in fact m68k/ELF and the start.? file
   for this system (sysdeps/m68k/elf/start.S) is also used by The Hurd
   and therefore this files must not contain the definition of the
   `errno' variable (I don't know why, ask Roland), we have to define
   it somewhere else.

   ...and this place is here.  */
	.bss
	.globl errno
	.type errno,@object
errno:	.space 4
	.size errno,4
weak_alias (errno, _errno)
	.text

/* The following code is only used in the shared library when we
   compile the reentrant version.  Otherwise each system call defines
   each own version.  */

#ifndef	PIC

/* The syscall stubs jump here when they detect an error.  */

#undef CALL_MCOUNT
#define CALL_MCOUNT /* Don't insert the profiling call, it clobbers %d0.  */

	.text
ENTRY (__syscall_error)
	neg.l %d0
#ifndef _LIBC_REENTRANT
	move.l %d0, errno
#else
	move.l %d0, -(%sp)
	jbsr __errno_location
	move.l (%sp)+, (%a0)
#endif
	move.l #-1, %d0
	/* Copy return value to %a0 for syscalls that are declared to
	   return a pointer.  */
	move.l %d0, %a0
	rts
END (__syscall_error)
#endif /* PIC */
