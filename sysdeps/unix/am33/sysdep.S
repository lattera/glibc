/* Copyright (C) 1991, 1992, 1993, 1994, 1995, 1996, 1997, 2000, 2001
   Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Alexandre Oliva <aoliva@redhat.com>.
   Based on ../i386/sysdep.S.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include <sysdep.h>
#define _ERRNO_H
#include <bits/errno.h>

.globl C_SYMBOL_NAME(errno)
.globl syscall_error

#undef syscall_error
#ifdef NO_UNDERSCORES
__syscall_error:
#else
syscall_error:
#endif
#if defined (EWOULDBLOCK_sys) && EWOULDBLOCK_sys != EAGAIN
	/* We translate the system's EWOULDBLOCK error into EAGAIN.
	   The GNU C library always defines EWOULDBLOCK==EAGAIN.
	   EWOULDBLOCK_sys is the original number.  */
	cmp EWOULDBLOCK_sys,d0	/* Is it the old EWOULDBLOCK?  */
	bne .Lnotb		/* Branch if not.  */
	mov EAGAIN,d0		/* Yes; translate it to EAGAIN.  */
.Lnotb:
#endif
#ifndef	PIC
# ifndef _LIBC_REENTRANT
	mov d0,(C_SYMBOL_NAME (errno))
# else
	movm [d2],(sp)
	add -12,sp
	mov d0,d2
	call __errno_location,[],0
	mov d2,(a0)
	add 12,sp
	movm (sp),[d2]
# endif
#else
# error "This shouldn't be assembled for PIC"
#endif
	mov -1,d0
	mov d0,a0
	ret

#undef	__syscall_error
END (__syscall_error)
