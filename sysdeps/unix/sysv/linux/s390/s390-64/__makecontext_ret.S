/* Copyright (C) 2012-2016 Free Software Foundation, Inc.
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

/* We do not want .eh_frame info so that __makecontext_ret stops unwinding
   if backtrace was called within a context created by makecontext. (There
   is also no .eh_frame info for _start or thread_start.)  */
#undef cfi_startproc
#define cfi_startproc
#undef cfi_endproc
#define cfi_endproc

ENTRY(__makecontext_ret)
	basr	%r14,%r7
	ltgr	%r8,%r8			/* Check whether uc_link is 0.  */
	jz	1f
	lgr	%r2,%r8
	br	%r9
1:	lghi	%r2,0
	brasl	%r14,HIDDEN_JUMPTARGET (exit)
	j	.+2			/* Trap if exit returns.  */
END(__makecontext_ret)
