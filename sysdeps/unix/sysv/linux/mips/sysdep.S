/* Copyright (C) 1998 Free Software Foundation, Inc.
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

/* Because the Linux version is in fact MIPS/ELF and the start.? file
   for this system (sysdeps/mips/elf/start.S) is also used by The Hurd
   and therefore this files must not contain the definition of the
   `errno' variable (I don't know why, ask Roland), we have to define
   it somewhere else.

   ...and this place is here.  */
	.bss
	.globl  errno
	.type   errno,@object
	.size   errno,4
errno:	.word   4
	.text
weak_alias(errno, _errno)

#include <sysdeps/unix/mips/sysdep.S>
