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

   You should have received a copy of the GNU General Public License
   along with GCC; see the file COPYING.  If not, write to the Free
   Software Foundation, 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.  */


	.section .rodata.str1.1,"aMS",%progbits,1
	.type	longjmp_msg,%object
longjmp_msg:
	.string "longjmp causes uninitialized stack frame"
	.size	longjmp_msg, .-longjmp_msg
	.text

#define __longjmp ____longjmp_chk

#ifdef PIC
# define CALL_FAIL						\
	ldr	sl, .L_GOT;					\
.L_GOT_OFF:							\
	add	sl, pc, sl;					\
	ldr	r0, .Lstr;					\
	add	r0, sl, r0;					\
	B	PLTJMP(HIDDEN_JUMPTARGET(__fortify_fail));	\
.L_GOT:								\
	.word	_GLOBAL_OFFSET_TABLE_-(.L_GOT_OFF+8);		\
.Lstr:								\
	.word	longjmp_msg(GOTOFF);
#else
# define CALL_FAIL					\
	ldr	r0, .Lstr;				\
	B	HIDDEN_JUMPTARGET(__fortify_fail);	\
.Lstr:							\
	.word	longjmp_msg;
#endif

#define CHECK_SP(reg)				\
	cmp	sp, reg;				\
	ble	.Lok;				\
	CALL_FAIL				\
.Lok:

#include <__longjmp.S>
