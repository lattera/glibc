/* BSD `setjmp' entry point to `sigsetjmp (..., 1)'.  s390 version.
   Copyright (C) 2000, 2001 Free Software Foundation, Inc.
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

/* This just does a tail-call to `__sigsetjmp (ARG, 1)'.
   We cannot do it in C because it must be a tail-call, so frame-unwinding
   in setjmp doesn't clobber the state restored by longjmp.  */

#include <sysdep.h>

ENTRY(setjmp)
#ifdef PIC
	/* We cannot use the PLT, because it requires that %r12 be set, but
	   we can't save and restore our caller's value.  Instead, we do an
	   indirect jump through the GOT. */
	basr   %r1,0
.L0:    al     %r1,.L1 - .L0(0,%r1) /* get address of global offset table */
				    /* get address of __sigjmp_save from got */
	l      %r1,__sigjmp_save@GOT12(0,%r1)
	lhi    %r3,1                /* second argument of one */
	br     %r1
.L1:    .long  _GLOBAL_OFFSET_TABLE_ - .L0
#else
	basr   %r1,0
.L0:    l      %r1,.L1 - .L0(0,%r1) /* load address of __sigsetjmp */
	lhi    %r3,1                /* second argument of zero */
	br     %r1                  /* branch to __sigsetjmp */
.L1:    .long  __sigsetjmp
#endif
END (setjmp)
