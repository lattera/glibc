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
	.globl _errno
	.type _errno,@object
_errno = errno	/* This name is expected by hj's libc.so.5 startup code.  */
	.text

/* The following code is only used in the shared library when we
   compile the reentrant version.  Otherwise each system call defines
   each own version.  */

#ifndef	PIC

/* The syscall stubs jump here when they detect an error.  */

	.globl	__syscall_error
	.type	__syscall_error, @function
	.align	4
__syscall_error:
	neg.l %d0
	move.l %d0, errno
#ifdef _LIBC_REENTRANT
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

ENTRY (__errno_location)
#ifdef PIC
	move.l	(%pc, errno@GOTPC), %a0
#else
	lea	errno, %a0
#endif
	rts
END (__errno_location)
