/* Startup code for Alpha/ELF.
Copyright (C) 1993, 1995, 1996 Free Software Foundation, Inc.
Contributed by Richard Henderson <rth@tamu.edu>

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

#include <sysdep.h>

	.text
	.align 3
	.globl _start
	.ent _start, 0
_start:
	.frame fp, 0, zero
	mov	zero, fp
	br	gp, 1f
1:	ldgp	gp, 0(gp)
	.prologue 1

  /* Save v0.  When starting a binary via the dynamic linker, s0
     contains the address of the shared library termination function,
     which we will register below with atexit() to be called by exit().
     If we are statically linked, this will be NULL.  */
	mov	v0, s0

  /* Do essential libc initialization (sp points to argc, argv, and envp)  */
	jsr	ra, __libc_init_first
	ldgp	gp, 0(ra)

  /* Now that we have the proper stack frame, register library termination
     function, if there is any:  */

	beq	s0, 1f
	mov	s0, a0
	jsr	ra, atexit
	ldgp	gp, 0(ra)
1:

  /* Extract the arguments and environment as encoded on the stack.  */
	ldl	a0, 0(sp)	/* get argc */
	lda	a1, 8(sp)	/* get argv */
	s8addq	a0, a1, a2	/* get envp */
	addq	a2, 8, a2
	stq	a2, _environ

	mov	a0, s0		/* tuck them away */
	mov	a1, s1
	mov	a2, s2

  /* Call _init, the entry point to our own .init section.  */
	jsr	ra, _init
	ldgp	gp, 0(ra)

  /* Register our .fini section with atexit.  */
	lda	a0, _fini
	jsr	ra, atexit
	ldgp	gp, 0(ra)

  /* Call the user's main and exit with its return value.  */
	mov	s0, a0
	mov	s1, a1
	mov	s2, a2

	jsr	ra, main
	ldgp	gp, 0(ra)

	mov	v0, a0
	jsr	ra, exit

  /* Die very horribly if exit returns.  Call_pal hlt is callable from
     kernel mode only; this will result in an illegal instruction trap.  */
	call_pal 0
	.end _start

/* For ECOFF backwards compatibility. */
weak_alias(_start, __start)

/* Define a symbol for the first piece of initialized data.  */
	.data
	.globl __data_start
__data_start:
	.long 0

#ifdef __ELF__
	.size __data_start, 4
	.type __data_start, @object
#endif

weak_alias(__data_start, data_start)
