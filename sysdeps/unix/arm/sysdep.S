/* Copyright (C) 1991,92,93,94,95,96,97,98 Free Software Foundation, Inc.
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

#ifndef	PIC
	ldr r1, _errno_loc
	str r0, [r1]
#ifdef _LIBC_REENTRANT
	stmdb sp!, {r0, lr}
	/* put another copy of r0 at a specific errno location */
	bl C_SYMBOL_NAME(__errno_location)
	ldmia sp!, {r1, lr}
	str r1, [r0]
#endif
#else
	stmdb sp!,{r10, lr}
	@ we have to establish our PIC register
	ldr r10, 1f
	add r10, pc, r10
0:	ldr r1, 2f
	ldr r1, [r10, r1]
	@ store a copy in _errno_loc
	str r0, [r1]
#ifdef _LIBC_REENTRANT
	@ and another copy in thread copy of _errno_loc
	mov r10, r0
	bl __errno_location(PLT)
	str r10, [r0]
#endif
	ldmia sp!, {r10, lr}
	b 4f
1:	.word _GLOBAL_OFFSET_TABLE_ - 0b - 4
2:	.word C_SYMBOL_NAME(errno)(GOT)
4:
#endif
	mvn r0, $0
	RETINSTR(mov, pc, r14)

#ifndef PIC
_errno_loc:	.long C_SYMBOL_NAME(errno)
#endif

#undef	__syscall_error
END (__syscall_error)
