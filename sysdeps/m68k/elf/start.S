/* Startup code compliant to the ELF m68k ABI.
Copyright (C) 1996 Free Software Foundation, Inc.
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
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

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
_start:
	/* Clear the frame pointer.  The ABI suggests this be done, to mark
	   the outermost frame obviously.  */
	sub.l %fp, %fp

	/* %a1 contains the address of the shared library termination
	   function, which we will register with `atexit' to be called by
	   `exit'.  */
	tstl %a1
	jbeq 1f
	move.l %a1, -(%sp)
	jbsr atexit
	addql #4, %sp
1:

	/* Do essential libc initialization.  In statically linked
	   programs under the GNU Hurd, this is what sets up the
	   arguments on the stack for the code below.  */
	jbsr __libc_init_first

	/* Extract the arguments and environment as encoded on the stack
	   and set up the arguments for `main': argc, argv, envp.  */
	move.l (%sp)+, %d0	/* Pop the argument count.  */
	lea (4,%sp,%d0*4), %a0	/* envp = &argv[argc + 1] */
	move.l %a0, _environ	/* Store it in the global variable.  */
	pea (%a0)		/* Push third argument: envp.  */
	pea 4(%sp)		/* Push second argument: argv.  */
	move.l %d0, -(%sp)	/* Push first argument: argc.  */

	/* Call `_init', which is the entry point to our own `.init'
	   section; and register with `atexit' to have `exit' call
	   `_fini', which is the entry point to our own `.fini' section.  */
	jbsr _init
	move.l #_fini, -(%sp)
	jbsr atexit
	addq.l #4, %sp

	/* Call the user's main function, and exit with its value.  */
	jbsr main
	move.l %d0, (%sp)
1:	jbsr exit		/* This should never return.  */
	jbra 1b			/* Try again if somehow it does return.  */

/* Define a symbol for the first piece of initialized data.  */
	.data
	.globl __data_start
__data_start:
	.long 0
	.weak data_start
	data_start = __data_start
