/* Startup code for SH & ELF.
   Copyright (C) 1999, 2001 Free Software Foundation, Inc.
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

/* This is the canonical entry point, usually the first thing in the text
   segment.

	Note that the code in the .init section has already been run.
	This includes _init and _libc_init


	At this entry point, most registers' values are unspecified, except:

   r4		Contains a function pointer to be registered with `atexit'.
		This is how the dynamic linker arranges to have DT_FINI
		functions called for shared libraries that have been loaded
		before this code runs.

   sp		The stack contains the arguments and environment:
		0(sp)			argc
		4(sp)			argv[0]
		...
		(4*argc)(sp)		NULL
		(4*(argc+1))(sp)	envp[0]
		...
					NULL
*/

	.text
	.globl _start
	.type _start,@function
_start:
	/* Clear the frame pointer since this is the outermost frame.  */
	mov #0, r14

	/* Pop argc off the stack and save a pointer to argv */
	mov.l @r15+,r5
	mov r15, r6

	/* Push the last arguments to main() onto the stack */
	mov.l r4,@-r15
	mov.l L_fini,r0
	mov.l r0,@-r15

	/* Set up the other arguments for main() that go in registers */
	mov.l L_main,r4
	mov.l L_init,r7

	/* __libc_start_main (main, argc, argv, init, fini, rtld_fini) */

	/* Let the libc call main and exit with its return code.  */
	mov.l L_libc_start_main,r1
	jsr @r1
	nop
	/* should never get here....*/
	mov.l L_abort,r1
	jsr @r1
	nop
	.align	2
L_main:
	.long	main
L_init:
	.long	_init
L_fini:
	.long	_fini
L_libc_start_main:
	.long	__libc_start_main
L_abort:
	.long	abort
/* Define a symbol for the first piece of initialized data.  */
	.data
	.globl __data_start
__data_start:
	.long 0
	.weak data_start
	data_start = __data_start
