/* Special .init and .fini section support for HPPA
   Copyright (C) 2000 Free Software Foundation, Inc.
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

/* If we use the standard C version, the linkage table pointer won't
   be properly preserved due to the splitting up of function prologues
   and epilogues.  Therefore we write these in assembly to make sure
   they do the right thing.

   Note that we cannot have a weak undefined __gmon_start__, because
   that would require this to be PIC, and the linker is currently not
   able to generate a proper procedure descriptor for _init.  Sad but
   true.  Anyway, HPPA is one of those horrible architectures where
   making the comparison and indirect call is quite expensive (see the
   comment in sysdeps/generic/initfini.c). */

__asm__ ("

#include \"defs.h\"

/*@HEADER_ENDS*/

/*@_init_PROLOG_BEGINS*/
	.section .init
	.align 4
	.globl _init
	.type _init,@function
_init:
	stw	%rp,-20(%sp)
	stwm	%r4,64(%sp)
	stw	%r19,-32(%sp)
	bl	__gmon_start__,%rp
	copy	%r19,%r4	/* delay slot */
	copy	%r4,%r19
/*@_init_PROLOG_ENDS*/

/*@_init_EPILOG_BEGINS*/
        .text
        .align 4
        .weak   __gmon_start__
        .type    __gmon_start__,@function
__gmon_start__:
	.proc
	.callinfo
	.entry
        bv,n %r0(%r2)
	.exit
	.procend

/* Here is the tail end of _init.  We put __gmon_start before this so
   that the assembler creates the .PARISC.unwind section for us, ie.
   with the right attributes.  */
	.section .init
	ldw	-84(%sp),%rp
	copy	%r4,%r19
	bv	%r0(%rp)
_end_init:
	ldwm	-64(%sp),%r4

/* Our very own unwind info, because the assembler can't handle
   functions split into two or more pieces.  */
	.section .PARISC.unwind
	.extern _init
	.word	_init, _end_init
	.byte	0x08, 0x01, 0x00, 0x08, 0x00, 0x00, 0x00, 0x08

/*@_init_EPILOG_ENDS*/

/*@_fini_PROLOG_BEGINS*/
	.section .fini
	.align 4
	.globl _fini
	.type _fini,@function
_fini:
	stw	%rp,-20(%sp)
	stwm	%r4,64(%sp)
	stw	%r19,-32(%sp)
	copy	%r19,%r4
/*@_fini_PROLOG_ENDS*/

/*@_fini_EPILOG_BEGINS*/
	.section .fini
	ldw	-84(%sp),%rp
	copy	%r4,%r19
	bv	%r0(%rp)
_end_fini:
	ldwm	-64(%sp),%r4

	.section .PARISC.unwind
	.extern _fini
	.word	_fini, _end_fini
	.byte	0x08, 0x01, 0x00, 0x08, 0x00, 0x00, 0x00, 0x08

/*@_fini_EPILOG_ENDS*/

/*@TRAILER_BEGINS*/
");
