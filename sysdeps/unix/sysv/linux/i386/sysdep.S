/* Copyright (C) 1995 Free Software Foundation, Inc.
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

/* The syscall stubs jump here when they detect an error.
   The code for Linux is almost identical to the canonical Unix/i386
   code, except that the error number in %eax is negated.  */

.globl __syscall_error
__syscall_error:
	negl %eax

#define __syscall_error __syscall_error_1
#include <sysdeps/unix/i386/sysdep.S>

/* Because the Linux version is in fact i386/ELF and the start.? file
   for this system (sysdeps/i386/elf/start.S) is also used by The Hurd
   and therefore this files must not contain the definition of the
   `errno' variable (I don't know why, ask Roland), we have to define
   it somewhere else.

   ...and this place is here.  */
	.data
	.globl errno
errno:
	.long 0
