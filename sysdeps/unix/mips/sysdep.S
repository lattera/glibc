/* Copyright (C) 1992,93,94,97,98,99,2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Brendan Kehoe (brendan@zen.org).

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
#define _ERRNO_H
#include <bits/errno.h>

#ifdef _LIBC_REENTRANT

ENTRY(__syscall_error)
#ifdef __PIC__
	.set noreorder
	.set	noat
	move	$1, $31
	bltzal	$0, 0f
	nop
0:	.cpload	$31
	move	$31, $1
	.set	at
	.set	reorder
#endif
	subu	sp, 32
#ifdef __PIC__
	.cprestore 16
#endif
	sw	v0, 20(sp)
	sw	ra, 24(sp)

#if defined (EWOULDBLOCK_sys) && EWOULDBLOCK_sys != EAGAIN
	/* We translate the system's EWOULDBLOCK error into EAGAIN.
	   The GNU C library always defines EWOULDBLOCK==EAGAIN.
	   EWOULDBLOCK_sys is the original number.  */
	bne	v0, EWOULDBLOCK_sys, skip
	nop
	li	v0, EAGAIN
skip:
#endif
	/* Store it in the "real" variable ... */
	sw v0, errno

	/* Find our per-thread errno address  */
	jal	__errno_location

	/* Store the error value.  */
	lw	t0, 20(sp)
	sw	t0, 0(v0)

	/* And just kick back a -1.  */
	lw	ra, 24(sp)
	addiu	sp, 32
	li	v0, -1
	j	ra
	END(__syscall_error)

#else /* _LIBC_REENTRANT */


ENTRY(__syscall_error)
#ifdef __PIC__
	.set	noreorder
	.set	noat
	move	$1, $31
	bltzal	$0, 0f
	nop
0:	.cpload	$31
	move	$31, $1
	.set	at
	.set	reorder
#endif
#if defined (EWOULDBLOCK_sys) && EWOULDBLOCK_sys != EAGAIN
	/* We translate the system's EWOULDBLOCK error into EAGAIN.
	   The GNU C library always defines EWOULDBLOCK==EAGAIN.
	   EWOULDBLOCK_sys is the original number.  */
	bne v0, EWOULDBLOCK_sys, skip
	li v0, EAGAIN
skip:
#endif
	/* Store it in errno... */
	sw v0, errno

	/* And just kick back a -1.  */
	li v0, -1
	j ra
	END(__syscall_error)
#endif  /* _LIBC_REENTRANT  */
