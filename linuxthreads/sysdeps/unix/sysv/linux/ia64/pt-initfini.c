/* Special .init and .fini section support for ia64. LinuxThreads version.
   Copyright (C) 2000, 2001 Free Software Foundation, Inc.
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

/*@_init_PROLOG_BEGINS*/
	.section .init
	.align 16
	.global _init#
	.proc _init#
_init:
	alloc r34 = ar.pfs, 0, 3, 0, 0
	mov r32 = r12
	mov r33 = b0
	adds r12 = -16, r12
	;;
/* we could use r35 to save gp, but we use the stack since that's what
 * all the other init routines will do --davidm 00/04/05 */
	st8 [r12] = gp, -16
	br.call.sptk.many b0 = __pthread_initialize_minimal# ;;
	;;
	adds r12 = 16, r12
	;;
	ld8 gp = [r12]
	;;
	.align 16
	.endp _init#

/*@_init_PROLOG_ENDS*/

/*@_init_EPILOG_BEGINS*/
	.section .init
	.regstk 0,2,0,0
	mov r12 = r32
	mov ar.pfs = r34
	mov b0 = r33
	br.ret.sptk.many b0
	.endp _init#
/*@_init_EPILOG_ENDS*/

/*@_fini_PROLOG_BEGINS*/
	.section .fini
	.align 16
	.global _fini#
	.proc _fini#
_fini:
	alloc r34 = ar.pfs, 0, 3, 0, 0
	mov r32 = r12
	mov r33 = b0
	adds r12 = -16, r12
	;;
	.align 16
	.endp _fini#

/*@_fini_PROLOG_ENDS*/

/*@_fini_EPILOG_BEGINS*/
	.section .fini
	mov r12 = r32
	mov ar.pfs = r34
	mov b0 = r33
	br.ret.sptk.many b0
	.endp _fini#

/*@_fini_EPILOG_ENDS*/

/*@TRAILER_BEGINS*/
	.weak	__gmon_start__#
");
