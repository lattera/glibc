/* Special .init and .fini section support for 64 bit S/390.
   Copyright (C) 2001 Free Software Foundation, Inc.
   Contributed by Martin Schwidefsky (schwidefsky@de.ibm.com).
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   In addition to the permissions in the GNU Library General Public
   License, the Free Software Foundation gives you unlimited
   permission to link the compiled version of this file with other
   programs, and to distribute those programs without any restriction
   coming from the use of this file.  (The Library General Public
   License restrictions do apply in other respects; for example, they
   cover modification of the file, and distribution when not linked
   into another program.)

   The GNU C Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* This file is compiled into assembly code which is then munged by a sed
   script into two files: crti.s and crtn.s.

   * crti.s puts a function prologue at the beginning of the
   .init and .fini sections and defines global symbols for
   those addresses, so they can be called as functions.

   * crtn.s puts the corresponding function epilogues
   in the .init and .fini sections. */

__asm__ ("

#include \"defs.h\"

/*@HEADER_ENDS*/

/*@TESTS_BEGIN*/

/*@TESTS_END*/

/*@_init_PROLOG_BEGINS*/

	.section .init
#NO_APP
	.align 4
.globl _init
	.type	 _init,@function
_init:
#	leaf function           0
#	automatics              0
#	outgoing args           0
#	need frame pointer      0
#	call alloca             0
#	has varargs             0
#	incoming args (stack)   0
#	function length         36
	STMG	6,15,48(15)
	LGR	1,15
	AGHI	15,-160
	STG	1,0(15)
	LARL	12,_GLOBAL_OFFSET_TABLE_
	BRASL	14,__pthread_initialize_minimal@PLT
	LARL	1,__gmon_start__@GOTENT
	LG	1,0(1)
	LTGR	1,1
	JE	.L22
	BASR	14,1
.L22:
#APP
	.align 4,0x07
	END_INIT

/*@_init_PROLOG_ENDS*/

/*@_init_EPILOG_BEGINS*/
	.align 4
	.section .init
#NO_APP
	.align 4
	LG	4,272(15)
	LMG	6,15,208(15)
	BR	4
#APP
	END_INIT

/*@_init_EPILOG_ENDS*/

/*@_fini_PROLOG_BEGINS*/
	.section .fini
#NO_APP
	.align 4
.globl _fini
	.type	 _fini,@function
_fini:
#	leaf function           0
#	automatics              0
#	outgoing args           0
#	need frame pointer      0
#	call alloca             0
#	has varargs             0
#	incoming args (stack)   0
#	function length         30
	STMG	6,15,48(15)
	LGR	1,15
	AGHI	15,-160
	STG	1,0(15)
	LARL	12,_GLOBAL_OFFSET_TABLE_
#APP
	.align 4,0x07
	END_FINI

/*@_fini_PROLOG_ENDS*/

/*@_fini_EPILOG_BEGINS*/
	.align 4
	.section .fini
#NO_APP
	.align 4
	LG	4,272(15)
	LMG	6,15,208(15)
	BR	4
#APP
	END_FINI

/*@_fini_EPILOG_ENDS*/

/*@TRAILER_BEGINS*/
");
