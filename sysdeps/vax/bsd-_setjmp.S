/* BSD `_setjmp' entry point to `sigsetjmp (..., 0)'.  Vax version.
   Copyright (C) 1994, 1997, 2002 Free Software Foundation, Inc.
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

/* This just does a tail-call to `__sigsetjmp (ARG, 0)'.
   We cannot do it in C because it must be a tail-call, so frame-unwinding
   in setjmp doesn't clobber the state restored by longjmp.  */

#include <sysdep.h>

ENTRY (_setjmp)
	popl r0			/* Pop return PC.  */
	popl r1			/* Pop jmp_buf argument.  */
	pushl $0		/* Push second argument of zero.  */
	pushl r1		/* Push back first argument.  */
	pushl r0		/* Push back return PC.  */
	jmp C_SYMBOL_NAME (__sigsetjmp)
libc_hidden_def (_setjmp)
