/* Startup code compliant to the ELF s390 ABI.
   Copyright (C) 2000, 2001, 2002, 2003, 2004 Free Software Foundation, Inc.
   Contributed by Martin Schwidefsky (schwidefsky@de.ibm.com).
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
	.type _start,@function
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
	l       %r6,.L2-.Llit(%r13)     # load pointer to __libc_csu_fini
	l       %r5,.L1-.Llit(%r13)     # load pointer to __libc_csu_init
	l       %r2,.L3-.Llit(%r13)     # load pointer to main
	l       %r1,.L4-.Llit(%r13)	# load pointer to __libc_start_main
#ifdef PIC
	l       %r12,.L5-.Llit(%r13)    # load .got pointer
	la	%r6,0(%r13,%r6)
	la	%r5,0(%r13,%r5)
	la	%r2,0(%r13,%r2)
	la	%r1,0(%r13,%r1)
	la	%r12,0(%r13,%r12)
#endif

	/* ok, now branch to the libc main routine */
	basr    %r14,%r1

	/* crash if __libc_start_main returns */
	.word   0

.Llit:
#ifndef PIC
.L1:    .long  __libc_csu_init
.L2:    .long  __libc_csu_fini
.L3:    .long  main
.L4:    .long  __libc_start_main
#else
.L1:    .long  __libc_csu_init-.Llit
.L2:    .long  __libc_csu_fini-.Llit
.L3:    .long  main-.Llit
.L4:    .long  __libc_start_main@plt-.Llit
.L5:    .long  _GLOBAL_OFFSET_TABLE_-.Llit
#endif

/* Define a symbol for the first piece of initialized data.  */
	.data
	.globl __data_start
__data_start:
	.long 0
	.weak data_start
	data_start = __data_start
