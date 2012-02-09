/* Copyright (C) 1994, 1997, 2012 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <sysdep.h>
#define _ERRNO_H
#include <bits/errno.h>

.global C_SYMBOL_NAME(errno)
.global syscall_error

.text
.align 2
__syscall_error:
#if defined (EWOULDBLOCK_sys) && EWOULDBLOCK_sys != EAGAIN
	/* We translate the system's EWOULDBLOCK error into EAGAIN.
	   The GNU C library always defines EWOULDBLOCK==EAGAIN.
	   EWOULDBLOCK_sys is the original number.  */
	cmp %o0, EWOULDBLOCK_sys
	be,a notblock
	mov EAGAIN, %o0
#endif
notblock: /* Store the error code in `errno'.  */
	sethi %hi(C_SYMBOL_NAME(errno)), %g1
	st %o0, [%g1 + %lo(C_SYMBOL_NAME(errno))]
	/* And return -1.  */
	retl
	mov -1, %o0
