/* Copyright (C) 1992, 1993, 1994, 1997, 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Brendan Kehoe (brendan@zen.org).

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
#define _ERRNO_H
#include <bits/errno.h>

	.bss
	.globl	errno
#ifdef __ELF__
	.type	errno, @object
#endif
	.size	errno, 4
errno:
	.space	4

weak_alias (errno, _errno)

	.set noreorder

ENTRY(__syscall_error)
#ifdef PIC
	.set	noat
	move	$1, $31
	bltzal	$0, 0f
	nop
0:	.cpload	$31
	move	$31, $1
	.set	at
#endif
#if defined (EWOULDBLOCK_sys) && EWOULDBLOCK_sys != EAGAIN
	/* We translate the system's EWOULDBLOCK error into EAGAIN.
	   The GNU C library always defines EWOULDBLOCK==EAGAIN.
	   EWOULDBLOCK_sys is the original number.  */
	bne v0, EWOULDBLOCK_sys, skip
	nop
	li v0, EAGAIN
skip:
#endif
	/* Store it in errno... */
	sw v0, errno

	/* And just kick back a -1.  */
	j ra
	li v0, -1
	END(__syscall_error)

/* We provide this alias for compatilility with other Unices
   like IRIX 5  */
weak_alias (__syscall_error, syscall_error)
