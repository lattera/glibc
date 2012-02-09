/* BSD `setjmp' entry point to `sigsetjmp (..., 1)'.  64 bit S/390 version.
   Copyright (C) 2001, 2002 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

/* This just does a tail-call to `__sigsetjmp (ARG, 0)'.
   We cannot do it in C because it must be a tail-call, so frame-unwinding
   in setjmp doesn't clobber the state restored by longjmp.  */

#include <sysdep.h>

ENTRY(_setjmp)
	slgr   %r3,%r3		    /* Second argument of zero.	 */
#ifdef PIC
	jg     __sigsetjmp@PLT	    /* Branch to PLT of __sigsetjmp.  */
#else
	jg     __sigsetjmp	    /* Branch to __sigsetjmp.  */
#endif
END (_setjmp)
libc_hidden_def (_setjmp)
