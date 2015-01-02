/* Copyright (C) 2012-2015 Free Software Foundation, Inc.
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

#include <sysdep.h>

ENTRY(__makecontext_ret)
	basr  %r14,%r7
	ltr   %r8,%r8			/* Check whether uc_link is 0.  */
	jz    1f
	lr    %r2,%r8
	br    %r9
1:	lhi   %r2,0			/* EXIT return value.  */
	basr	%r13,0
2:
#ifdef PIC
	l       %r12,4f-2b(%r13)
	la      %r12,0(%r12,%r13)	/* GOT pointer in r12 after this.  */
	l       %r1,3f-2b(%r13)
	bas     %r14,0(%r1,%r12)
	.align  4
3:
	.long   HIDDEN_JUMPTARGET (exit)@GOTOFF
4:
	.long   _GLOBAL_OFFSET_TABLE_-2b
#else
	l	%r1,3f-2b(%r13)
	basr	%r14,%r1
	.align  4
3:
	.long   HIDDEN_JUMPTARGET (exit)
#endif
	.align	2
	j	.+2			/* Trap if exit returns for some reason.  */
END(__makecontext_ret)
