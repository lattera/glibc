/* Special .init and .fini section support for SH.
   Copyright (C) 2000, 2002, 2009 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   In addition to the permissions in the GNU Lesser General Public
   License, the Free Software Foundation gives you unlimited
   permission to link the compiled version of this file with other
   programs, and to distribute those programs without any restriction
   coming from the use of this file. (The GNU Lesser General Public
   License restrictions do apply in other respects; for example, they
   cover modification of the file, and distribution when not linked
   into another program.)

   Note that people who make modified versions of this file are not
   obligated to grant this special exception for their modified
   versions; it is their choice whether to do so. The GNU Lesser
   General Public License gives permission to release a modified
   version without this exception; this exception also makes it
   possible to release a modified version which carries forward this
   exception.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

/* This file is compiled into assembly code which is then munged by a sed
   script into two files: crti.s and crtn.s.

   * crti.s puts a function prologue at the beginning of the
   .init and .fini sections and defines global symbols for
   those addresses, so they can be called as functions.

   * crtn.s puts the corresponding function epilogues
   in the .init and .fini sections. */

__asm__ ("\
\n\
#include \"defs.h\"\n\
\n\
/*@HEADER_ENDS*/\n\
\n\
/*@TESTS_BEGIN*/\n\
	.align 5\n\
/*@TESTS_END*/\n\
\n\
/*@_init_PROLOG_BEGINS*/\n\
	.section	.init,\"ax\",@progbits\n\
	.align 5\n\
	.global	_init\n\
	.type	_init, @function\n\
_init:\n\
	mov.l	r12,@-r15\n\
	mova	.L12,r0\n\
	mov.l	.L12,r12\n\
	mov.l	r14,@-r15\n\
	add	r0,r12\n\
	mov.l	.L13,r0\n\
	sts.l	pr,@-r15\n\
	mov.l	@(r0,r12),r1\n\
	tst	r1,r1\n\
	bt/s	.L8\n\
	mov	r15,r14\n\
	mov.l	.L14,r1\n\
	bsrf	r1\n\
.LPCS0:\n\
	nop\n\
.L8:\n\
	bra	1f\n\
	nop\n\
	.align 2\n\
.L12:\n\
	.long	_GLOBAL_OFFSET_TABLE_\n\
.L13:\n\
	.long	__gmon_start__@GOT\n\
.L14:\n\
	.long	__gmon_start__@PLT-(.LPCS0+2-(.))\n\
1:\n\
	ALIGN\n\
	END_INIT\n\
\n\
/*@_init_PROLOG_ENDS*/\n\
\n\
/*@_init_EPILOG_BEGINS*/\n\
	.section .init\n\
	mov	r14,r15\n\
	lds.l	@r15+,pr\n\
	mov.l	@r15+,r14\n\
	mov.l	@r15+,r12\n\
	rts	\n\
	nop\n\
	END_INIT\n\
\n\
/*@_init_EPILOG_ENDS*/\n\
\n\
/*@_fini_PROLOG_BEGINS*/\n\
	.section	.fini,\"ax\",@progbits\n\
	.align 5\n\
	.global	_fini\n\
	.type	_fini, @function\n\
_fini:\n\
	mov.l	r12,@-r15\n\
	mova	.L19,r0\n\
	mov.l	r14,@-r15\n\
	sts.l	pr,@-r15\n\
	mov.l	.L19,r12\n\
	mov	r15,r14\n\
	add	r0,r12\n\
	bra	0f\n\
	nop\n\
	.align 2\n\
.L19:\n\
	.long	_GLOBAL_OFFSET_TABLE_\n\
0:\n\
	ALIGN\n\
	END_FINI\n\
\n\
/*@_fini_PROLOG_ENDS*/\n\
	mov.l	.L20,r1\n\
	bsrf	r1\n\
.LPCS1:\n\
	nop\n\
	bra	1f\n\
	nop\n\
	.align 2\n\
.L20:\n\
	.long	i_am_not_a_leaf@PLT-(.LPCS1+2-(.))\n\
1:\n\
/*@_fini_EPILOG_BEGINS*/\n\
	.section .fini\n\
	mov	r14,r15\n\
	lds.l	@r15+,pr\n\
	mov.l	@r15+,r14\n\
	mov.l	@r15+,r12\n\
	rts	\n\
	nop\n\
	END_FINI\n\
\n\
/*@_fini_EPILOG_ENDS*/\n\
\n\
/*@TRAILER_BEGINS*/\n\
	.weak	__gmon_start__\n\
");
