/* BSD `setjmp' entry point to `sigsetjmp (..., 1)'.  i386 version.
Copyright (C) 1994, 1995 Free Software Foundation, Inc.
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
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

/* This just does a tail-call to `__sigsetjmp (ARG, 1)'.
   We cannot do it in C because it must be a tail-call, so frame-unwinding
   in setjmp doesn't clobber the state restored by longjmp.  */

#include <sysdep.h>

ENTRY (setjmp)
	popl %eax		/* Pop return PC.  */
	popl %ecx		/* Pop jmp_buf argument.  */
	pushl $1		/* Push second argument of one.  */
	pushl %ecx		/* Push back first argument.  */
	pushl %eax		/* Push back return PC.  */
	jmp JUMPTARGET (C_SYMBOL_NAME (__sigsetjmp))
