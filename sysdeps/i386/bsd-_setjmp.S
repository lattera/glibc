/* BSD `_setjmp' entry point to `sigsetjmp (..., 0)'.  i386 version.
   Copyright (C) 1994, 1995, 1996, 1997 Free Software Foundation, Inc.
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

/* This just does a tail-call to `__sigsetjmp (ARG, 0)'.
   We cannot do it in C because it must be a tail-call, so frame-unwinding
   in setjmp doesn't clobber the state restored by longjmp.  */

#include <sysdep.h>

ENTRY (_setjmp)
	popl %eax		/* Pop return PC.  */
	popl %ecx		/* Pop jmp_buf argument.  */
	pushl $0		/* Push second argument of zero.  */
	pushl %ecx		/* Push back first argument.  */
	pushl %eax		/* Push back return PC.  */
#ifdef	PIC
	/* We cannot use the PLT, because it requires that %ebx be set, but
           we can't save and restore our caller's value.  Instead, we do an
           indirect jump through the GOT, using for the temporary register
           %ecx, which is call-clobbered.  */
	call here
here:	popl %ecx
	addl $_GLOBAL_OFFSET_TABLE_+[.-here], %ecx
	movl C_SYMBOL_NAME(__sigsetjmp@GOT)(%ecx), %ecx
	jmp *%ecx
#else
	jmp __sigsetjmp
#endif
END (_setjmp)
