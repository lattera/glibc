/* Startup code compliant to the ELF i386 ABI.
Copyright (C) 1995 Free Software Foundation, Inc.
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
   segment.  The SVR4/i386 ABI (pages 3-31, 3-32) says that when the entry
   point runs, most registers' values are unspecified, except for:

   %edx		Contains a function pointer to be registered with `atexit'.
   		This is how the dynamic linker arranges to have DT_FINI
		functions called for shared libraries that have been loaded
		before this code runs.

   %esp		The stack contains the arguments and environment:
   		0(%esp)			argc
		4(%esp)			argv[0]
		...
		(4*argc)(%esp)		NULL
		(4*(argc+1))(%esp)	envp[0]
		...
					NULL
*/

	.text	
	.globl _start
_start:
	/* Clear the frame pointer.  The ABI suggests this be done, to mark
	   the outermost frame obviously.  */
	movl $0, %ebp

	/* %edx contains the address of the shared library termination
	   function, which we will register with `atexit' to be called by
	   `exit'.  I suspect that on some systems, and when statically
	   linked, this will not be set by anything to any function
	   pointer; hopefully it will be zero so we don't try to call
	   random pointers.  */
	testl %edx
	jeq nofini
	pushl %edx
	call atexit
	addl $4, %esp
nofini:

	/* Do essential libc initialization.  In statically linked
	   programs under the GNU Hurd, this is what sets up the
	   arguments on the stack for the code below.  */
	call __libc_init_first

	/* Extract the arguments and environment as encoded on the stack
	   and set up the arguments for `main': argc, argv, envp.  */
	popl %esi		/* Pop the argument count.  */
	leal 4(%esp,%esi,4), %eax /* envp = &argv[argc + 1] */
	movl %eax, _environ	/* Store it in the global variable.  */
	pushl %eax		/* Push third argument: envp.  */
	leal 4(%esp), %eax	/* argv starts just above that word.  */
	pushl %eax		/* Push second argument: argv.  */
	pushl %esi		/* Push first argument: argc.  */

	/* Call `_init', which is the entry point to our own `.init'
	   section; and register with `atexit' to have `exit' call
	   `_fini', which is the entry point to our own `.fini' section.  */
	call _init
	pushl $_fini
	call atexit
	addl $4, %esp

	/* Call the user's main function, and exit with its value.  */
	call main
	pushl %eax
	call exit		/* This should never return.  */
	hlt			/* Crash if somehow it does return.  */
