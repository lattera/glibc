/* Startup code compliant to the ELF m68k ABI.
   Copyright (C) 1996, 1997, 1998, 2001 Free Software Foundation, Inc.
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

/* This is the canonical entry point, usually the first thing in the text
   segment.  The SVR4/m68k ABI says that when the entry point runs,
   most registers' values are unspecified, except for:

   %a1		Contains a function pointer to be registered with `atexit'.
		This is how the dynamic linker arranges to have DT_FINI
		functions called for shared libraries that have been loaded
		before this code runs.

   %sp		The stack contains the arguments and environment:
		0(%sp)			argc
		4(%sp)			argv[0]
		...
		(4*argc)(%sp)		NULL
		(4*(argc+1))(%sp)	envp[0]
		...
					NULL
*/

	.text
	.globl _start
	.type _start,@function
_start:
	/* Clear the frame pointer.  The ABI suggests this be done, to mark
	   the outermost frame obviously.  */
	sub.l %fp, %fp

	/* Extract the arguments as encoded on the stack and set up the
	   arguments for `main': argc, argv.  envp will be determined
	   later in __libc_start_main.  */
	move.l (%sp)+, %d0	/* Pop the argument count.  */
	move.l %sp, %a0		/* The argument vector starts just at the
				   current stack top.  */

	/* Provide the highest stack address to the user code (for stacks
	   which grow downward).  */
	pea (%sp)

	pea (%a1)		/* Push address of the shared library
				   termination function.  */

	/* Push the address of our own entry points to `.fini' and
	   `.init'.  */
	pea _fini
	pea _init

	pea (%a0)		/* Push second argument: argv.  */
	move.l %d0, -(%sp)	/* Push first argument: argc.  */

	pea main

	/* Call the user's main function, and exit with its value.  But
	   let the libc call main.  */
	jbsr __libc_start_main

	illegal			/* Crash if somehow `exit' does return.  */

/* Define a symbol for the first piece of initialized data.  */
	.data
	.globl __data_start
__data_start:
	.long 0
	.weak data_start
	data_start = __data_start
