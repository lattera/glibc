/* Copyright (C) 1992, 1993, 1995 Free Software Foundation, Inc.
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

/* In the Linux/ELF world, C symbols are asm symbols.  */
#define NO_UNDERSCORES

/* There is some commonality.  */
#include <sysdeps/unix/i386/sysdep.h>

#ifdef ASSEMBLER

/* Linux uses a negative return value to indicate syscall errors, unlike
   most Unices, which use the condition codes' carry flag.  */
#undef	PSEUDO
#define	PSEUDO(name, syscall_name, args)				      \
  .text;								      \
  .globl __syscall_error;						      \
  ENTRY (name)								      \
    movl $SYS_##syscall_name, %eax;					      \
    DO_CALL (args)							      \
    testl %eax, %eax;							      \
    jl JUMPTARGET(__syscall_error)


/* Linux takes system call arguments in registers:

	syscall number	%eax	     call-clobbered
	arg 1		%ebx	     call-saved
	arg 2		%ecx	     call-clobbered
	arg 3		%edx	     call-clobbered
	arg 4		%esi	     call-saved
	arg 5		%edi	     call-saved

   The stack layout upon entering the function is:

	24(%esp)	Arg# 5
	20(%esp)	Arg# 4
	16(%esp)	Arg# 3
	12(%esp)	Arg# 2
	 8(%esp)	Arg# 1
	 4(%esp)	Return address
	  (%esp)

   (Of course a function with e.g. 3 argumentS does not have entries for
   arguments 4 and 5.)

   We put the arguments into registers from the stack, and save the
   call-saved registers, by using the 386 `xchg' instruction to swap the
   values in both directions.  */

#undef	DO_CALL
#define DO_CALL(args)					      		      \
    DOARGS_##args							      \
    int $0x80;								      \
    UNDOARGS_##args							      \

#define	DOARGS_0	/* No arguments to frob.  */
#define	UNDOARGS_0	/* No arguments to unfrob.  */
#define	DOARGS_1	xchg 8(%esp), %ebx; DOARGS_0 /* Save %ebx on stack.  */
#define	UNDOARGS_1	xchg 8(%esp), %ebx; UNDOARGS_0 /* Restore %ebx */
#define	DOARGS_2	movel 12(%esp), %ecx; DOARGS_1
#define	UNDOARGS_2	UNDOARGS_1 /* %ecx is clobbered.  */
#define	DOARGS_3	movel 16(%esp), %edx; DOARGS_2
#define	UNDOARGS_3	UNDOARGS_2 /* %edx is clobbered.  */
#define	DOARGS_4	xchg 20(%esp), %esi; DOARGS_3 /* Save %esi on stack. */
#define	UNDOARGS_4	xchg 20(%esp), %esi; UNDOARGS_3	/* Restore %esi.  */
#define	DOARGS_5	xchg 24(%esp), %edi; DOARGS_3 /* Save %edi on stack. */
#define	UNDOARGS_5	xchg 24(%esp), %edi; UNDOARGS_3	/* Restore %edi.  */


#endif	/* ASSEMBLER */
