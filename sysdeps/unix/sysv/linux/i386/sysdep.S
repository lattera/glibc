/* Copyright (C) 1995, 1996 Free Software Foundation, Inc.
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

/* Because the Linux version is in fact i386/ELF and the start.? file
   for this system (sysdeps/i386/elf/start.S) is also used by The Hurd
   and therefore this files must not contain the definition of the
   `errno' variable (I don't know why, ask Roland), we have to define
   it somewhere else.

   ...and this place is here.  */
	.bss
	.globl errno
	.type errno,@object
	.size errno,4
errno:	.zero 4
	.globl _errno
	.type _errno,@object
_errno = errno	/* This name is expected by hj libc.so.5 startup code.  */
	.globl __errno
	.type __errno,@object
__errno = errno	/* This name is expected by the MT code.  */
	.text

/* The following code is only used in the shared library when we
   compile the reentrant version.  Otherwise each system call defines
   each own version.  */

#ifndef PIC

/* The syscall stubs jump here when they detect an error.
   The code for Linux is almost identical to the canonical Unix/i386
   code, except that the error number in %eax is negated.  */

ENTRY (__syscall_error)
	negl %eax

#define __syscall_error __syscall_error_1
#include <sysdeps/unix/i386/sysdep.S>

#endif	/* !PIC */


ENTRY (__errno_location)
#ifdef PIC
	call .L2
.L2:	popl %ecx
	addl $_GLOBAL_OFFSET_TABLE_+[.-.L2], %ecx
	movl errno@GOT(%ecx), %eax
#else
	movl $errno, %eax
#endif
	ret
END (__errno_location)
