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

/* In Linux the `stat' call is actually done by emulating a `xstat' system
   call, which takes an additional first argument giving a version number
   for `struct stat'.  Likewise for `fstat' and `lstat' there are `fxstat'
   and `lxstat' emulations.  This macro gives the Linux version number that
   corresponds to the definition of `struct stat' in <statbuf.h>.  */
#define	_STAT_VER	1

ENTRY (__stat)
	movl 8(%esp), %eax
	movl 4(%esp), %ecx
#ifdef	PIC
	pushl %ebx
	call 0f
0:	popl %ebx
	addl $_GLOBAL_OFFSET_TABLE_+[.-0b], %ebx
#endif
	pushl %eax
	pushl %ecx
	pushl $_STAT_VER	/* Push extra first arg to syscall.  */
	call JUMPTARGET(__xstat)/* Jump to xstat implementation.  */
	addl $12, %esp
#ifdef	PIC
	popl %ebx
#endif
	ret

weak_alias (__stat, stat)
