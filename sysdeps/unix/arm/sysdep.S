/* Copyright (C) 1991, 1992, 1993, 1994, 1995, 1996, 1997, 1998, 2002, 2003,
   2004, 2005, 2009, 2011
   Free Software Foundation, Inc.
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
#define _ERRNO_H
#include <bits/errno.h>

#ifdef IS_IN_rtld
# include <dl-sysdep.h>			/* Defines RTLD_PRIVATE_ERRNO.  */
#endif

#include <tls.h>

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
	cmp r0, $EWOULDBLOCK_sys /* Is it the old EWOULDBLOCK?  */
	moveq r0, $EAGAIN	/* Yes; translate it to EAGAIN.  */
#endif

#ifndef IS_IN_rtld
	mov ip, lr
	cfi_register (lr, ip)
	mov r1, r0

	mov r0, #0xffff0fff
	mov lr, pc
	sub pc, r0, #31

	ldr r2, 1f
2:	ldr r2, [pc, r2]
	str r1, [r0, r2]
	mvn r0, #0
	RETINSTR (, ip)

1:	.word errno(gottpoff) + (. - 2b - 8)
#elif RTLD_PRIVATE_ERRNO
	ldr r1, 1f
0:	str r0, [pc, r1]
	mvn r0, $0
	DO_RET(r14)

1:	.word C_SYMBOL_NAME(rtld_errno) - 0b - 8
#else
#error "Unsupported non-TLS case"
#endif

#undef	__syscall_error
END (__syscall_error)
