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

#include <sysdep.h>

/* In Linux the `mknod' call is actually done by a emulation of the
   `xmknod' system call, which takes an additional first argument
   giving a version number for the interface.  This macro gives the
   Linux version number that corresponds to the modern interface.  */
#define _MKNOD_VER      1

ENTRY (__mknod)
	movl 12(%esp), %edx
	movl 8(%esp), %ecx
	movl 4(%esp), %eax
	pushl %edx
	pushl %ecx
	pushl %eax
        pushl $_MKNOD_VER       /* Push extra first arg to syscall.  */
	call JUMPTARGET(__xmknod)
	addl $16, %esp		/* Correct version parameter offset.  */
	ret

weak_alias (__mknod, mknod)
