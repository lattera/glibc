/* Copyright (C) 2000, 2001, 2002, 2003, 2004 Free Software Foundation, Inc.
   Contributed by Martin Schwidefsky (schwidefsky@de.ibm.com).
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
#include <tls.h>

/* The following code is only used in the shared library when we
   compile the reentrant version.  Otherwise each system call defines
   each own version.  */

/* The syscall stubs jump here when they detect an error.  */

#undef CALL_MCOUNT
#define CALL_MCOUNT

	.text
ENTRY(__syscall_error)
#ifndef PIC
# if USE___THREAD
#  ifndef NOT_IN_libc
#   define SYSCALL_ERROR_ERRNO __libc_errno
#  else
#   define SYSCALL_ERROR_ERRNO errno
#  endif
	basr	%r1,0
0:	l	%r1,1f-0b(%r1)
	ear	%r3,%a0
	lcr	%r2,%r2
	st	%r2,0(%r1,%r3)
	lhi	%r2,-1
	br	%r14
1:	.long	SYSCALL_ERROR_ERRNO@ntpoff
# elif !defined _LIBC_REENTRANT
	basr	%r1,0
0:	l	%r1,1f-0b(%r1)
	lcr	%r2,%r2
	st	%r2,0(%r1)
	lhi	%r2,-1
	br	%r14
1:	.long  errno
# else
	stm	%r13,%r15,52(%r15)
	cfi_offset (%r15, -36)
	cfi_offset (%r14, -40)
	cfi_offset (%r13, -44)
	lr	%r0,%r15
	ahi	%r15,-96
	cfi_adjust_cfa_offset (96)
	lcr	%r13,%r2
	st	%r0,0(%r15)
	basr	%r1,0
0:	l	%r1,1f-0b(%r1)
	basr	%r14,%r1
	st	%r13,0(%r2)
	lm	%r13,%r15,148(%r15)
	cfi_adjust_cfa_offset (-96)
	lhi	%r2,-1
	br	%r14
1:	.long  __errno_location
#endif
#else
# if RTLD_PRIVATE_ERRNO
	basr	%r1,0
0:	al	%r1,1f-0b(%r1)
	lcr	%r2,%r2
	st	%r2,0(%r1)
	lhi	%r2,-1
	br	%r14
1:	.long	rtld_errno - 0b
# elif USE___THREAD
#  ifndef NOT_IN_libc
#   define SYSCALL_ERROR_ERRNO __libc_errno
#  else
#   define SYSCALL_ERROR_ERRNO errno
#  endif
	basr	%r1,0
0:	al	%r1,1f-0b(%r1)
	ear	%r3,%a0
	l	%r1,SYSCALL_ERROR_ERRNO@gotntpoff(%r1)
	lcr	%r2,%r2
	st	%r2,0(%r1,%r3)
	lhi	%r2,-1
	br	%r14
1:	.long	_GLOBAL_OFFSET_TABLE_-0b
# elif !defined _LIBC_REENTRANT
	basr	%r1,0
0:	al	%r1,1f-0b(%r1)
	l	%r1,errno@GOT(%r1)
	lcr	%r2,%r2
	st	%r2,0(0,%r1)
	lhi	%r2,-1
	br	%r14
1:	.long	_GLOBAL_OFFSET_TABLE_-0b
# else
	stm	%r11,%r15,44(%r15)
	cfi_offset (%r15, -36)
	cfi_offset (%r14, -40)
	cfi_offset (%r13, -44)
	cfi_offset (%r12, -48)
	cfi_offset (%r11, -52)
	lr	%r0,%r15
	ahi	%r15,-96
	cfi_adjust_cfa_offset (96)
	lcr	%r11,%r2
	st	%r0,0(%r15)
	basr	%r13,0
0:	l	%r12,1f-0b(%r13)
	l	%r1,2f-0b(%r13)
	la	%r12,0(%r12,%r13)
	bas	%r14,0(%r1,%r13)
	st	%r11,0(%r2)
	lm	%r11,%r15,140(%r15)
	cfi_adjust_cfa_offset (-96)
	lhi	%r2,-1
	br	%r14
1:	.long _GLOBAL_OFFSET_TABLE_-0b
2:	.long __errno_location@PLT-0b
# endif
#endif

END (__syscall_error)
