/* Startup code compliant to the ELF s390 ABI.
   Copyright (C) 2000, 2001 Free Software Foundation, Inc.
   Contributed by Martin Schwidefsky (schwidefsky@de.ibm.com).
   This file is part of the GNU C Library.

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

/*
   This is the canonical entry point, usually the first thing in the text
   segment. Most registers' values are unspecified, except for:

   %r14         Contains a function pointer to be registered with `atexit'.
		This is how the dynamic linker arranges to have DT_FINI
		functions called for shared libraries that have been loaded
		before this code runs.

   %r15		The stack contains the arguments and environment:
		0(%r15)			argc
		4(%r15)			argv[0]
		...
		(4*argc)(%r15)		NULL
		(4*(argc+1))(%r15)	envp[0]
		...
					NULL
*/

	.text
	.globl _start
_start:
	/* Setup pointer to literal pool of _start */
	basr    %r13,0
.L0:    ahi     %r13,.Llit-.L0

	/* load argc and argv from stack */
	la      %r4,4(%r15)             # get argv
	l       %r3,0(%r15)             # get argc

	/* align the stack to a double word boundary */
	lhi     %r0,-8
	nr      %r15,%r0

	/* Setup a stack frame and a parameter area */
	ahi     %r15,-104               # make room on stack
	xc      0(4,%r15),0(%r15)       # clear back-chain

	/* set up arguments for __libc_start_main:
	   main, argc, argv, envp, _init, _fini, rtld_fini, stack_end
	   Note that envp will be determined later in __libc_start_main
	 */
	stm     %r14,%r15,96(%r15)      # store rtld_fini/stack_end to parameter area
	la      %r7,96(%r15)
	l       %r6,.L2-.Llit(%r13)     # load pointer to _fini
	l       %r5,.L1-.Llit(%r13)     # load pointer to _init
	l       %r2,.L3-.Llit(%r13)     # load pointer to main

	/* ok, now branch to the libc main routine */
	l       %r1,.L4-.Llit(%r13)
	basr    %r14,%r1

	/* crash if __libc_start_main returns */
	.word   0

.Llit:
.L1:    .long  _init
.L2:    .long  _fini
.L3:    .long  main
.L4:    .long  __libc_start_main

/* FIXME: FPU flags or what ?!? */

	.section .rodata
	.globl _fp_hw
	.long 3
	.size _fp_hw, 4

/* Define a symbol for the first piece of initialized data.  */
	.data
	.globl __data_start
__data_start:
	.long 0
	.weak data_start
	data_start = __data_start
