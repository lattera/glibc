/* Copyright (C) 2001 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

	.file	"start.s"
	.toc
T.lsd:	.tc __libc_start_data[tc], __libc_start_data[rw]
T.main:	.tc main[tc], main[rw]
T.init:	.tc __libc_start_init[tc], __libc_start_init[rw]
T.fini:	.tc __libc_start_fini[tc], __libc_start_init[rw]
T.rtld_fini :	 .tc __libc_start_rtld_fini[tc], __libc_start_rtld_fini[rw]

	.globl __start
	.globl .__start
	.globl __libc_start_data

	.extern .__libc_start_main
	.extern .main
	.extern main
	.extern __libc_start_init
	.extern __libc_start_fini
	.extern __libc_start_rtld_fini

/* Text */

	.csect __start[ds]
__start:
	.long .__start, TOC[tc0], 0

	.csect .text[pr]
.__start:

/* No prologue needed, __start does not have to follow the ABI.

 Input from kernel/loader
	r1 :	stack
	r2 :	TOC
	r3 :	argc
	r4 :	argv
	r5 :	envp
	r28 :	data origin
	r29 :	text origin
	r30 :	module count
	r31 :	default processing flag

	If r31 == r30, no special processing is needed, ie r28, r29 & r30
	are not used

 Save input in __libc_start_data */
	l	16, T.lsd(2)
	st	1,  0(16)	/* stack */
	st	2,  4(16)	/* toc */
	st	3,  8(16)	/* argc */
	st	4,  12(16)	/* argv */
	st	5,  16(16)	/* envp */
	st	28, 20(16)	/* data origin */
	st	29, 24(16)	/* text origin */
	st	30, 28(16)	/* module count */
	st	31, 32(16)	/* special */

/* Call __libc_start_main() */

	bl	.__libc_start_main
	nop

/* No epilog needed, __start does not have to follow the ABI */

/* Trace back */
TB.__start:
	.long 0x0
	.long 0xc2040
	.long 0x0
	.long TB.__start - .__start
	.short 7
	.byte "__start"
	.byte 0,0,0

/* Data
 __libc_start_data
 Space to keep libc initialization information */

	.csect __libc_start_data[rw]
__libc_start_data:
/* For kernel/loader input args	*/
	.space 36

/* Externs */
	.long main
init:
	.long __libc_start_init
fini:
	.long __libc_start_fini
rtld_fini:
	.long __libc_start_rtld_fini
	.space 0x1000 + (4 + rtld_fini - __libc_start_data)
