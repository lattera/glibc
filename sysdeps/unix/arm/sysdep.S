/* Copyright (C) 1991,92,93,94,95,96,97,98,2002,03 Free Software Foundation, Inc.
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
	cmp r0, $EWOULDBLOCK_sys /* Is it the old EWOULDBLOCK?  */
	moveq r0, $EAGAIN	/* Yes; translate it to EAGAIN.  */
#endif

#ifdef _LIBC_REENTRANT
	str lr, [sp, #-4]!
	str r0, [sp, #-4]!
	bl PLTJMP(C_SYMBOL_NAME(__errno_location))
	ldr r1, [sp], #4
	str r1, [r0]
	mvn r0, $0
	ldr pc, [sp], #4	
#else
#ifndef	PIC
	ldr r1, 1f
	str r0, [r1]
	mvn r0, $0
	DO_RET (r14)

1:	.long C_SYMBOL_NAME(errno)
#else
	@ we have to establish our PIC register
	ldr r2, 1f
	ldr r1, 2f
0:	add r2, pc, r2
	str r0, [r1, r2]
	mvn r0, $0
	DO_RET (r14)

1:	.word _GLOBAL_OFFSET_TABLE_ - 0b - 8
2:	.word C_SYMBOL_NAME(errno)(GOTOFF)
#endif
#endif

#undef	__syscall_error
END (__syscall_error)
