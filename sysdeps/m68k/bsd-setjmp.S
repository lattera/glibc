/* BSD `setjmp' entry point to `sigsetjmp (..., 1)'.  m68k version.
   Copyright (C) 1994, 1997 Free Software Foundation, Inc.
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

/* This just does a tail-call to `__sigsetjmp (ARG, 1)'.
   We cannot do it in C because it must be a tail-call, so frame-unwinding
   in setjmp doesn't clobber the state restored by longjmp.  */

#include <sysdep.h>

#ifdef MOTOROLA_SYNTAX
#define d0 %d0
#define d1 %d1
#define PUSH(reg)	move.l reg, -(%sp)
#define POP(reg)	move.l (%sp)+, reg
#else
#define PUSH(reg)	movel reg, sp@-
#define POP(reg)	movel sp@+, reg
#endif

ENTRY (setjmp)
	POP (d0)		/* Pop return PC.  */
	POP (d1)		/* Pop jmp_buf argument.  */
	pea 1			/* Push second argument of one.  */
	PUSH (d1)		/* Push back first argument.  */
	PUSH (d0)		/* Push back return PC.  */
#ifdef PIC
	bra.l C_SYMBOL_NAME (__sigsetjmp@PLTPC)
#else
	jmp C_SYMBOL_NAME (__sigsetjmp)
#endif
END (setjmp)
