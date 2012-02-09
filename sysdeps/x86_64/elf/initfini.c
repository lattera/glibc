/* Special .init and .fini section support for x86-64.
   Copyright (C) 2001, 2002 Free Software Foundation, Inc.
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

__asm__ ("\n\
#include \"defs.h\"\n\
\n\
/*@HEADER_ENDS*/\n\
\n\
/*@_init_PROLOG_BEGINS*/\n\
	.align 4\n\
	.type	call_gmon_start,@function\n\
call_gmon_start:\n\
	subq	$8, %rsp\n\
	movq	__gmon_start__@GOTPCREL(%rip), %rax\n\
	testq	%rax, %rax\n\
	je	.L22\n\
	call	*%rax\n\
.L22:\n\
	addq	$8, %rsp\n\
	ret\n\
\n\
	.section .init\n\
	.align 4\n\
.globl _init\n\
	.type	_init,@function\n\
_init:\n\
	subq	$8, %rsp\n\
	call	call_gmon_start\n\
	ALIGN\n\
	END_INIT\n\
\n\
/*@_init_PROLOG_ENDS*/\n\
\n\
/*@_init_EPILOG_BEGINS*/\n\
	.section .init\n\
	addq	$8, %rsp\n\
	ret\n\
	END_INIT\n\
\n\
/*@_init_EPILOG_ENDS*/\n\
\n\
/*@_fini_PROLOG_BEGINS*/\n\
	.section .fini\n\
	.align 4\n\
.globl _fini\n\
	.type	_fini,@function\n\
_fini:\n\
	subq	$8, %rsp\n\
	ALIGN\n\
	END_FINI\n\
\n\
/*@_fini_PROLOG_ENDS*/\n\
	call	i_am_not_a_leaf@PLT\n\
\n\
/*@_fini_EPILOG_BEGINS*/\n\
	.section .fini\n\
	addq	$8, %rsp\n\
	ret\n\
	END_FINI\n\
\n\
/*@_fini_EPILOG_ENDS*/\n\
\n\
/*@TRAILER_BEGINS*/\n\
	.weak	__gmon_start__\n\
");
