/* Copyright (C) 2001, 2002 Free Software Foundation, Inc.
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

/* The Linux version is in fact x86-64/ELF and the start.? file for this
   system (sysdeps/x86_64/elf/start.S) is also used by The Hurd.  This file
   must not contain the definition of the `errno' variable, we have to
   define it somewhere else.

   ...and this place is here.  */
#if USE_TLS && HAVE___THREAD
	.section .tbss
#else
	.bss
#endif
	.globl	errno
	.type errno,@object
	.size errno,4
	.globl	_errno
	.type _errno,@object
	.size _errno,4
	.align 4
errno:
_errno:
	.space	4


/* The following code is only used in the shared library when we
   compile the reentrant version.  Otherwise each system call defines
   each own version.  */

#ifndef PIC

/* The syscall stubs jump here when they detect an error.
   The code for Linux is almost identical to the canonical Unix
   code, except that the error number in %rax is negated.  */

#undef CALL_MCOUNT
#define CALL_MCOUNT /* Don't insert the profiling call, it clobbers %rax.  */

	.text
ENTRY (__syscall_error)
	negq %rax

#define __syscall_error __syscall_error_1
#include <sysdeps/unix/x86_64/sysdep.S>

#endif	/* !PIC */
