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
_errno = errno	/* This name is expected by hj libc.so.5 startup code.  */
	.text

/* The following code is not used at all in the shared library.
   The PIC system call stubs set errno themselves.  */

#ifndef	PIC

#include <sysdep.h>
#define _ERRNO_H
#include <errnos.h>

.globl errno
.globl __syscall_error

/* The syscall stubs jump here when they detect an error.  */

.globl __syscall_error
__syscall_error:
	neg.l %d0

#if defined (EWOULDBLOCK_sys) && EWOULDBLOCK_sys != EAGAIN
	/* We translate the system's EWOULDBLOCK error into EAGAIN.
	   The GNU C library always defines EWOULDBLOCK==EAGAIN.
	   EWOULDBLOCK_sys is the original number.  */
	move.l #EWOULDBLOCK_sys, %d1
	cmp.l %d0, %d1
	jne 1f
	move.l #EAGAIN, %d0
1:
#endif

	move.l %d0, errno
	move.l #-1, %d0
	/* Copy return value to %a0 for syscalls that are declared to
	   return a pointer.  */
	move.l %d0, %a0
	rts
#endif /* PIC */
