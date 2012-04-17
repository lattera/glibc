/* Copyright (C) 2009 Free Software Foundation, Inc.
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

	.section .rodata.str1.8,"aMS",@progbits,1
	.align 8
.LC0:
	.string "longjmp causes uninitialized stack frame"

	.section .sdata,"aws",@progbits
	.align 8
	.type	longjmp_msg,@object
longjmp_msg:
	data8	.LC0
	.size	longjmp_msg, .-longjmp_msg

#define __longjmp ____longjmp_chk

#define CHECK_RSP(reg) \
	cmp.ltu p0, p8 = reg, r12;				\
(p8)	br.cond.dpnt .Lok;;					\
	addl r28 = @ltoffx(longjmp_msg#), r1;;			\
	ld8.mov r28 = [r28], longjmp_msg#;;			\
	ld8 out0 = [r28];					\
	br.call.sptk.many b0 = HIDDEN_JUMPTARGET(__fortify_fail)#;; \
.Lok:

#include "__longjmp.S"
