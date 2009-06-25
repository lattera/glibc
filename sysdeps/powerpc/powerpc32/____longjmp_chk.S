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

#ifdef PIC
# ifdef HAVE_ASM_PPC_REL16
#  define LOAD_ARG \
	bcl	20,31,1f;				\
1:	mflr	r3;					\
	addis	r3,r3,_GLOBAL_OFFSET_TABLE_-1b@ha;	\
	addi	r3,r3,_GLOBAL_OFFSET_TABLE_-1b@l;	\
	lwz	r3,.LC0@got(r3)
# else
#  define LOAD_ARG \
	bl	_GLOBAL_OFFSET_TABLE_-4@local;		\
	mflr	r3;					\
	lwz	r3,.LC0@got(r3)
# endif
#else
# define LOAD_ARG \
	lis	r3,.LC0@ha;				\
	la	r3,.LC0@l(r3)
#endif

#define CHECK_SP(reg) \
	cmplw	reg, r1;				\
	bge+	.Lok;					\
	LOAD_ARG;					\
	bl	HIDDEN_JUMPTARGET (__fortify_fail);	\
.Lok:

#include <__longjmp-common.S>
