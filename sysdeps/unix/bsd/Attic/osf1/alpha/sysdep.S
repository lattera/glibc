/* Copyright (C) 1993 Free Software Foundation, Inc.
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
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <sysdep.h>
#define _ERRNO_H
#include <errnos.h>

ENTRY(syscall_error)
#ifdef EWOULDBLOCK_sys
	/* We translate the system's EWOULDBLOCK error into EAGAIN.
	   The GNU C library always defines EWOULDBLOCK==EAGAIN.
	   EWOULDBLOCK_sys is the original number.  */
	subq v0, EWOULDBLOCK_sys, t0
	cmoveq t0, EAGAIN, v0
#endif

	/* Store it in errno... */
!	ldgp gp, 0(t12)
	stl v0, errno

	/* And just kick back a -1.  */
	ldil v0, -1
	ret

	.end syscall_error
