/* Copyright (C) 1991-2015 Free Software Foundation, Inc.
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

#if IS_IN (rtld)
# include <dl-sysdep.h>		/* Defines RTLD_PRIVATE_ERRNO.  */
#endif

.globl C_SYMBOL_NAME(errno)
.globl syscall_error

__syscall_error:
#if defined (EWOULDBLOCK_sys) && EWOULDBLOCK_sys != EAGAIN
	/* We translate the system's EWOULDBLOCK error into EAGAIN.
	   The GNU C library always defines EWOULDBLOCK==EAGAIN.
	   EWOULDBLOCK_sys is the original number.  */
	cmpl $EWOULDBLOCK_sys, %eax /* Is it the old EWOULDBLOCK?  */
	jne notb		/* Branch if not.  */
	movl $EAGAIN, %eax	/* Yes; translate it to EAGAIN.  */
notb:
#endif
#ifndef	PIC
# ifndef NO_TLS_DIRECT_SEG_REFS
	movl %eax, %gs:C_SYMBOL_NAME(errno@NTPOFF)
# else
	movl %gs:0, %ecx
	movl %eax, C_SYMBOL_NAME(errno@NTPOFF)(%ecx)
# endif
#else
	/* The caller has pushed %ebx and then set it up to
	   point to the GOT before calling us through the PLT.  */
	movl C_SYMBOL_NAME(errno@GOTNTPOFF)(%ebx), %ecx

	/* Pop %ebx value saved before jumping here.  */
	popl %ebx
# ifndef NO_TLS_DIRECT_SEG_REFS
	addl %gs:0, %ecx
	movl %eax, (%ecx)
# else
	movl %eax, %gs:0(%ecx)
# endif
#endif
	movl $-1, %eax
	ret

#undef	__syscall_error
END (__syscall_error)
