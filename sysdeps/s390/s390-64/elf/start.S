/* Startup code compliant to the 64 bit S/390 ELF ABI.
   Copyright (C) 2001 Free Software Foundation, Inc.
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

   %r14		Contains a function pointer to be registered with `atexit'.
		This is how the dynamic linker arranges to have DT_FINI
		functions called for shared libraries that have been loaded
		before this code runs.

   %r15		The stack contains the arguments and environment:
		0(%r15)			argc
		8(%r15)			argv[0]
		...
		(8*argc)(%r15)		NULL
		(8*(argc+1))(%r15)	envp[0]
		...
					NULL
*/

	.text
	.globl _start
	.type _start,@function
_start:
	/* Load argc and argv from stack.  */
	la	%r4,8(%r15)		# get argv
	lg	%r3,0(%r15)		# get argc

	/* Align the stack to a double word boundary.  */
	lghi	%r0,-16
	ngr	%r15,%r0

	/* Setup a stack frame and a parameter area.  */
	aghi	%r15,-176		# make room on stack
	xc	0(8,%r15),0(%r15)	# clear back-chain

	/* Set up arguments for __libc_start_main:
	   main, argc, argv, envp, _init, _fini, rtld_fini, stack_end
	   Note that envp will be determined later in __libc_start_main.
	 */
	stmg	%r14,%r15,160(%r15)	# store rtld_fini/stack_end to parameter area
	la	%r7,160(%r15)
	larl	%r6,_fini		# load pointer to _fini
	larl	%r5,_init		# load pointer to _init
	larl	%r2,main		# load pointer to main

	/* Ok, now branch to the libc main routine.  */
	brasl	%r14,__libc_start_main

	/* Crash if __libc_start_main returns.	*/
	.word	0

	/* FIXME: FPU flags or what ?!?	 */

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
