/* Copyright (C) 2001, 2002, 2004 Free Software Foundation, Inc.
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
#include <tls.h>

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
	cmpq $EWOULDBLOCK_sys, %rax /* Is it the old EWOULDBLOCK?  */
	jne notb		/* Branch if not.  */
	movq $EAGAIN, %rax	/* Yes; translate it to EAGAIN.  */
notb:
#endif
#if USE___THREAD
# ifdef PIC
	movq C_SYMBOL_NAME(errno@GOTTPOFF)(%rip), %rcx
	movl %eax, %fs:0(%rcx)
# else
	movl %eax, %fs:C_SYMBOL_NAME(errno@TPOFF)
# endif
#elif !defined PIC
# ifndef _LIBC_REENTRANT
	movl %eax, C_SYMBOL_NAME(errno)
# else
	pushq %rax
	cfi_adjust_cfa_offset(8)
	PUSH_ERRNO_LOCATION_RETURN
	call BP_SYM (__errno_location)
	POP_ERRNO_LOCATION_RETURN
	popq %rcx
	cfi_adjust_cfa_offset(-8)
	movl %ecx, (%rax)
# endif
#else
# if RTLD_PRIVATE_ERRNO
	leaq rtld_errno(%rip), %rcx
	movl %eax, (%rcx)
# elif !defined _LIBC_REENTRANT
	movq C_SYMBOL_NAME(errno)@GOTPCREL(%rip), %rcx
	movl %eax, (%rcx)
# else
	pushq %rax
	cfi_adjust_cfa_offset(8)
	PUSH_ERRNO_LOCATION_RETURN
	call C_SYMBOL_NAME (BP_SYM (__errno_location)@PLT)
	POP_ERRNO_LOCATION_RETURN
	popq %rcx
	cfi_adjust_cfa_offset(-8)
	movl %ecx, (%rax)
# endif
#endif
	movq $-1, %rax
	ret

#undef	__syscall_error
END (__syscall_error)
