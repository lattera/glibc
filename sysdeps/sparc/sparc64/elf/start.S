/* Startup code for elf64-sparc
   Copyright (C) 1997 Free Software Foundation, Inc.
   Contributed by Richard Henderson <richard@gnu.ai.mit.edu>, 1997.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <sysdep.h>


	.section ".text"
	.align 4
	.global _start
	.type _start,#function
_start:

  /* First order of business is to load %g4 with our base address.  */
	sethi	%uhi(_start), %g4
	or	%g4, %ulo(_start), %g4
	sllx	%g4, 32, %g4

  /* Terminate the stack frame, and reserve space for functions to
     drop their arguments.  */
	mov	%g0, %fp
	sub	%sp, 6*8, %sp

  /* Save %g1.  When starting a binary via the dynamic linker, %g1
     contains the address of the shared library termination function,
     which we will register below with atexit() to be called by exit().
     If we are statically linked, this will be NULL.  */

  /* Do essential libc initialization (sp points to argc, argv, and envp)  */
	call	__libc_init_first
	 mov	%g1, %l0

  /* Now that we have the proper stack frame, register library termination
     function, if there is any:  */

	brz,pn	%l0, 1f
	 nop
	call	atexit
	 mov	%l0, %o0
1:

  /* Extract the arguments and environment as encoded on the stack.  The
     argument info starts after one register window (16 words) past the SP,
     plus the bias we added, plus the magic v9 STACK_BIAS.  */
	ldx	[%sp+STACK_BIAS+22*8], %o0
	add	%sp, STACK_BIAS+23*8, %o1
	sllx	%o0, 3, %o2
	add	%o2, %o1, %o2
	sethi	%hi(__environ), %g2
	add	%o2, 8, %o2
	add	%g2, %g4, %g2
	stx	%o2, [%g2+%lo(__environ)]

	mov	%o0, %l0		/* tuck them away */
	mov	%o1, %l1

  /* Call _init, the entry point to our own .init section.  */
	call	_init
	 mov	%o2, %l2

  /* Register our .fini section with atexit.  */
	sethi	%hi(_fini), %o0
	add	%o0, %g4, %o0
	call	atexit
	 add	%o0, %lo(_fini), %o0

  /* Call the user's main and exit with its return value.  */
	mov	%l0, %o0
	mov	%l1, %o1
	call	main
	 mov	%l2, %o2
	call	exit
	 nop

  /* Die very horribly if exit returns.  */
	illtrap	0

	.size _start,.-_start
