/* Special .init and .fini section support for SH.
   Copyright (C) 2000 Free Software Foundation, Inc.
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

/* This file is compiled into assembly code which is then munged by a sed
   script into two files: crti.s and crtn.s.

   * crti.s puts a function prologue at the beginning of the
   .init and .fini sections and defines global symbols for
   those addresses, so they can be called as functions.

   * crtn.s puts the corresponding function epilogues
   in the .init and .fini sections. */

__asm__ ("

#include \"defs.h\"
#define SHARED

/*@HEADER_ENDS*/

/*@TESTS_BEGIN*/

/*@TESTS_END*/

/*@_init_PROLOG_BEGINS*/
	.section .init
	.align 5
	.global	_init
	.type	_init,@function
_init:
	mov.l	r12,@-r15
	mov.l	r14,@-r15
	sts.l	pr,@-r15
#ifdef SHARED
	mova	.L22,r0
	mov.l	.L22,r12
	add	r0,r12
	mova	.L23,r0
	mov.l	.L23,r1
	add	r0,r1
#else
	mov.l	.L23,r1
#endif
	jsr	@r1
	 mov	r15,r14
	bra	1f
	 nop
	.align 2
#ifdef SHARED
.L22:
	.long	_GLOBAL_OFFSET_TABLE_
.L23:
	.long	__gmon_start__@PLT
#else
.L23:
	.long	__gmon_start__
#endif
	.data
	.global __fpscr_values
__fpscr_values:
	.long   0
	.long   0x80000
	.previous
1:
	ALIGN
	END_INIT

	
/*@_init_PROLOG_ENDS*/

/*@_init_EPILOG_BEGINS*/
	.section .init
	mov	r14,r15
	lds.l	@r15+,pr
	mov.l	@r15+,r14
	rts	
	mov.l	@r15+,r12
	END_INIT
	.section .text
	.align 5
	.weak	__gmon_start__
	.type	__gmon_start__,@function
__gmon_start__:
	mov.l	r14,@-r15
	mov	r15,r14
	mov	r14,r15
	rts	
	mov.l	@r15+,r14
	
/*@_init_EPILOG_ENDS*/

/*@_fini_PROLOG_BEGINS*/
	.section .fini
	.align 5
	.global	_fini
	.type	_fini,@function
_fini:
	mov.l	r12,@-r15
	mov.l	r14,@-r15
	sts.l	pr,@-r15
#ifdef SHARED
	mova	.L27,r0
	mov.l	.L27,r12
	add	r0,r12
#endif
	mov	r15,r14
	ALIGN
	END_FINI
#ifdef SHARED
	bra	1f
	 nop
	.align	2
.L27:
	.long	_GLOBAL_OFFSET_TABLE_
#endif
1:
/*@_fini_PROLOG_ENDS*/

/*@_fini_EPILOG_BEGINS*/
	.section .fini
	mov	r14,r15
	lds.l	@r15+,pr
	mov.l	@r15+,r14
	rts	
	mov.l	@r15+,r12

	END_FINI
	
/*@_fini_EPILOG_ENDS*/

/*@TRAILER_BEGINS*/
");
