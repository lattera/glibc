/* ELF startup code for HPPA.
   Copyright (C) 2002 Free Software Foundation, Inc.
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

	.text

	.align 4

	.import main, code
	.import $global$, data
	.import __libc_start_main, code
	.import __libc_csu_fini, code
	.import __libc_csu_init, code

	.globl _start
	.export _start, ENTRY
	.type _start,@function
_start:

	.proc
	.callinfo

	/* load main */
	ldil	LP%main, %r26
	ldo	RP%main(%r26), %r26

	/* argc and argv should be in 25 and 24 */

	/* Expand the stack to store the 5th through 7th args */
	ldo	64(%sp), %sp

	/* void (*rtld_fini) (void) (actually the 6th arg) */
	stw	%r23, -56(%sp)

	/* void (*init) (void) */
	ldil	LP%__libc_csu_init, %r23
	ldo	RP%__libc_csu_init(%r23), %r23

	/* void (*fini) (void) */
	ldil	LP%__libc_csu_fini, %r22
	ldo	RP%__libc_csu_fini(%r22), %r22
	stw	%r22, -52(%sp)

	/* void *stack_end */
	stw	%sp, -60(%sp)

	/* load global */
	ldil	L%$global$, %dp
	ldo	R%$global$(%dp), %dp

	bl	__libc_start_main,%r2
	nop
	/* die horribly if it returned (it shouldn't) */
	iitlbp %r0,(%r0)
	nop

	.procend

/* Define a symbol for the first piece of initialized data.  */
	.data
	.globl __data_start
__data_start:
	.long 0
	.weak data_start
	data_start = __data_start
