/* Copyright (C) 1996 Free Software Foundation, Inc.
   Contributed by Miguel de Icaza (miguel@nuclecu.unam.mx)

   FIXME:	Currently only 6 parameters can be passsed
	        to the clone function.
		
   based on code written for the Intel by Richard 
   Henderson (rth@tamu.edu)
	
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

/* clone() is even more special than fork() as it mucks with stacks
   and invokes a function in the right context after its all over.  */
	
#include <asm/errno.h>
#include <asm/unistd.h>

/* int clone(int (*fn)(), void *child_stack, int flags, int nargs, ...) */

	.text
	.align	4
	.globl	__libc_clone
	.type	__libc_clone,@function
	.weak	clone
	__clone = __libc_clone
	clone   = __libc_clone
__libc_clone:	
	save	%sp,-96,%sp
	/* sanity check arguments */
	tst	%i0
	be	__clone_syscall_error
	tst	%i1
	be	__clone_syscall_error
	cmp	%i3,0
	bl	__clone_syscall_error
        nop

	/* Flush the register windows */
	ta	3

	/* Parameters for system call */	
	mov	%i1,%o1
	mov	%i2,%o0

	/* parameters that are on the stack for routine */
#define P(x) 0x5c+(x*4)
	ld	[%fp+P(0)],%o2
	ld	[%fp+P(1)],%o3
	ld	[%fp+P(2)],%o4
	ld	[%fp+P(3)],%o5
	/* Do the system call */
	set	__NR_clone,%g1
	ta	0x10
	bcs	__clone_syscall_error
	tst	%o1
	bne	__thread_start
	nop
	mov	%o0,%i0
	ret
	restore
	
__clone_syscall_error:
	call	__errno_location
	set	EINVAL,%i0
	st	%i0,[%o0]
	mov	-1,%i0
	ret
	restore

__thread_start:
	mov	%i5,%o1
	call	%i0
	mov	%i4,%o0
	call	_exit,0
	nop
