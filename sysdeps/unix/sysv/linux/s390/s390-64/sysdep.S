/* Copyright (C) 2001 Free Software Foundation, Inc.
   Contributed by Martin Schwidefsky (schwidefsky@de.ibm.com).
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <sysdep.h>

/* Because the Linux version is in fact m68k/ELF and the start.? file
   for this system (sysdeps/m68k/elf/start.S) is also used by The Hurd
   and therefore this files must not contain the definition of the
   `errno' variable (I don't know why, ask Roland), we have to define
   it somewhere else.

    ...and this place is here.  */
	.bss
	.globl	errno
	.type errno,@object
	.size errno,4
errno:
	.space	4
weak_alias (errno, _errno)

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
	larl    %r1,errno
	st      %r2,0(%r1)
	lghi    %r2,-1
	br      %r14
#else
        stmg    %r13,%r15,104(%r15)
        lgr     %r0,%r15
        aghi    %r15,-160
        lcr     %r13,%r2
        stg     %r0,0(%r15)
	brasl   %r14,__errno_location
	st      %r13,0(%r2)
	lmg     %r13,%r15,264(%r15)
	lghi    %r2,-1
	br      %r14
#endif
#else
#ifndef _LIBC_REENTRANT
	larl    %r1,_GLOBAL_OFFSET_TABLE_
	lg      %r1,errno@GOT(%r1)
	lcr     %r2,%r2
	st      %r2,0(%r1)
	lghi    %r2,-1
	br      %r14
#else
        stmg    %r13,%r15,104(%r15)
        lgr     %r0,%r15
        aghi    %r15,-160
        lcr     %r13,%r2
        stg     %r0,0(%r15)
	brasl   %r14,__errno_location@PLT
	st      %r13,0(%r2)
	lmg     %r13,%r15,264(%r15)
	lghi    %r2,-1	
	br      %r14
#endif
#endif

END (__syscall_error)

