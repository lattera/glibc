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

#include <sysdep.h>
#include <rtld-global-offsets.h>

	.section .rodata.str1.1,"aMS",@progbits,1
.LC0:
	.string "longjmp causes uninitialized stack frame"
	.text

#define __longjmp ____longjmp_chk

#define CHECK_SP(reg) \
	cmplw	reg, r1;				\
	bge+	.Lok;					\
	lis	r3,.LC0@ha;				\
	la	r3,.LC0@l(r3);				\
	bl	HIDDEN_JUMPTARGET (__fortify_fail);	\
.Lok:

#include <__longjmp-common.S>
