/* Copyright (C) 1991,1992,1993,1994,1995,1996,1997,2000,2002,2004,2005
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
#include <bp-asm.h>
#include <bp-sym.h>

#ifdef IS_IN_rtld
# include <dl-sysdep.h>		/* Defines RTLD_PRIVATE_ERRNO.  */
#endif

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
	cmpl $EWOULDBLOCK_sys, %eax /* Is it the old EWOULDBLOCK?  */
	jne notb		/* Branch if not.  */
	movl $EAGAIN, %eax	/* Yes; translate it to EAGAIN.  */
notb:
#endif
#ifndef	PIC
# if USE___THREAD
#  ifndef NO_TLS_DIRECT_SEG_REFS
	movl %eax, %gs:C_SYMBOL_NAME(errno@NTPOFF)
#  else
	movl %gs:0, %ecx
	movl %eax, C_SYMBOL_NAME(errno@NTPOFF)(%ecx)
#  endif
# elif !defined _LIBC_REENTRANT
	movl %eax, C_SYMBOL_NAME(errno)
# else
	pushl %eax
	PUSH_ERRNO_LOCATION_RETURN
	call BP_SYM (__errno_location)
	POP_ERRNO_LOCATION_RETURN
	popl %ecx
	movl %ecx, (%eax)
# endif
#else
	/* The caller has pushed %ebx and then set it up to
	   point to the GOT before calling us through the PLT.  */
# if USE___THREAD
	movl C_SYMBOL_NAME(errno@GOTNTPOFF)(%ebx), %ecx

	/* Pop %ebx value saved before jumping here.  */
	popl %ebx
#  ifndef NO_TLS_DIRECT_SEG_REFS
	addl %gs:0, %ecx
	movl %eax, (%ecx)
#  else
	movl %eax, %gs:0(%ecx)
#  endif
# elif RTLD_PRIVATE_ERRNO
	movl %eax, C_SYMBOL_NAME(rtld_errno@GOTOFF)(%ebx)

	/* Pop %ebx value saved before jumping here.  */
	popl %ebx
# elif !defined _LIBC_REENTRANT
	movl C_SYMBOL_NAME(errno@GOT)(%ebx), %ecx

	/* Pop %ebx value saved before jumping here.  */
	popl %ebx
	movl %eax, (%ecx)
# else
	pushl %eax
	PUSH_ERRNO_LOCATION_RETURN
	call C_SYMBOL_NAME (BP_SYM (__errno_location)@PLT)
	POP_ERRNO_LOCATION_RETURN
	popl %ecx
	/* Pop %ebx value saved before jumping here.  */
	popl %ebx
	movl %ecx, (%eax)
# endif
#endif
	movl $-1, %eax
	ret

#undef	__syscall_error
END (__syscall_error)
