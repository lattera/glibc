/* Copyright (C) 1993, 1994, 1997 Free Software Foundation, Inc.
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

ENTRY(syscall_error)
	/* If it was a syscall that got interrupted, but can
	   be restarted, drop EINTR in.  */
	cmp %o0, ERESTART
	be,a notint
	mov EINTR, %o0

notint:
#if defined (EWOULDBLOCK_sys) && EWOULDBLOCK_sys != EAGAIN
	/* We translate the system's EWOULDBLOCK error into EAGAIN.
	   The GNU C library always defines EWOULDBLOCK==EAGAIN.
	   EWOULDBLOCK_sys is the original number.  */
	cmp %o0, EWOULDBLOCK_sys
	be,a notblock
	mov EAGAIN, %o0
#endif

notblock:/* Store it in errno... */
	sethi %hi(C_SYMBOL_NAME(errno)), %g1
	st %o0, [%g1 + %lo(C_SYMBOL_NAME(errno))]

	/* And just kick back a -1.  */
	retl
	mov -1, %o0
