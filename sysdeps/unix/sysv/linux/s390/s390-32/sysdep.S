/* Copyright (C) 2000, 2001, 2002 Free Software Foundation, Inc.
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

/* The following code is only used in the shared library when we
   compile the reentrant version.  Otherwise each system call defines
   each own version.  */

/* The syscall stubs jump here when they detect an error.  */

#undef CALL_MCOUNT
#define CALL_MCOUNT

	.text
ENTRY(__syscall_error)
#ifndef PIC
#ifndef _LIBC_REENTRANT
	lcr     %r2,%r2
	basr    %r1,0
.L0:    l       %r1,.L1-.L0(%r1)
	st      %r2,0(0,%r1)
	lhi     %r2,-1
	br      %r14
.L1:    .long  errno
#else
	stm     %r11,%r15,44(%r15)
	lr      %r0,%r15
	ahi     %r15,-96
	st      %r0,0(%r15)
	lcr     %r11,%r2
	basr    %r13,0
.L0:	l       %r1,.L1-.L0(%r13)
	basr    %r14,%r1
	st      %r11,0(%r2)
	lhi     %r2,-1
	l       %r15,0(%r15)
	lm      %r11,%r15,44(%r15)
	br      %r14
.L1:	.long  __errno_location
#endif
#else
#ifndef _LIBC_REENTRANT
	basr    %r1,0
.L0:    al      %r1,.L1-.L0(%r1)
	l       %r1,errno@GOT12(%r1)
	lcr     %r2,%r2
	st      %r2,0(0,%r1)
	lhi     %r2,-1
	br      %r14
.L1:    .long   _GLOBAL_OFFSET_TABLE_-0b
#else
	stm     %r11,%r15,44(%r15)
	lr      %r0,%r15
	ahi     %r15,-96
	st      %r0,0(%r15)
	lcr     %r11,%r2
	basr    %r13,0
.L0:	l       %r12,.L1-.L0(%r13)
	ar      %r12,%r13
	l       %r14,.L2-.L0(%r13)
	bas     %r14,0(%r14,%r13)
	st      %r11,0(0,%r2)
	lhi     %r2,-1
	l       %r15,0(%r15)
	lm      %r11,%r15,44(%r15)
	br      %r14
.L1:	.long _GLOBAL_OFFSET_TABLE_ - .L0
.L2:    .long __errno_location@PLT - .L0
#endif
#endif

END (__syscall_error)
