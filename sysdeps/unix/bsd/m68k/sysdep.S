/* Copyright (C) 1991, 1992, 1993, 1994, 1997 Free Software Foundation, Inc.
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

#define _ERRNO_H
#include <bits/errno.h>

.globl syscall_error
syscall_error:
	/* We translate the system's EWOULDBLOCK error into EAGAIN.
	   The GNU C library always defines EWOULDBLOCK==EAGAIN.
	   EWOULDBLOCK_sys is the original number.  */
#ifdef __motorola__
#if defined (EWOULDBLOCK_sys) && EWOULDBLOCK_sys != EAGAIN
	cmp.l #EWOULDBLOCK_sys, d0
	bne store
	moveq.l #EAGAIN, d0
#endif
store:	move.l d0, _errno
	moveq.l #-1, d0
#else
#if defined (EWOULDBLOCK_sys) && EWOULDBLOCK_sys != EAGAIN
	cmpl #EWOULDBLOCK_sys, d0
	bne store
	moveq #EAGAIN, d0
#endif
store:	movel d0, _errno
	moveq #-1, d0
#endif
	rts
