/* Copyright (C) 1992, 1993, 1995 Free Software Foundation, Inc.
This file is part of the GNU C Library.
Contributed by Ulrich Drepper, <drepper@gnu.ai.mit.edu>, August 1995.

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

/* For Linux we can use the system call table in the header file
	/usr/include/asm/unistd.h
   of the kernel.  But these symbols do not follow the SYS_* syntax
   so we have to redefine the `SYS_ify' macro here.  */
#undef SYS_ify
#ifdef __STDC__
# define SYS_ify(syscall_name)	__NR_##syscall_name
#else
# define SYS_ify(syscall_name)	__NR_/**/syscall_name
#endif


#ifdef ASSEMBLER

/* Linux uses a negative return value to indicate syscall errors, unlike
   most Unices, which use the condition codes' carry flag.  */
#undef	PSEUDO
#define	PSEUDO(name, syscall_name, args)				      \
  .text;								      \
  .globl syscall_error;							      \
  ENTRY (name)								      \
    movl $SYS_ify (syscall_name), %eax;					      \
    DO_CALL (args);							      \
    testl %eax, %eax;							      \
    jl JUMPTARGET (syscall_error)

/* We define our own ENTRY macro because the alignment should be 16 for ELF.  */
#undef ENTRY
#define ENTRY(name)							      \
  ASM_GLOBAL_DIRECTIVE C_SYMBOL_NAME (name);				      \
  ASM_TYPE_DIRECTIVE (C_SYMBOL_NAME (name), @function)			      \
  .align 16;								      \
  C_LABEL (name)

/* Linux takes system call arguments in registers:

	syscall number	%eax	     call-clobbered
	arg 1		%ebx	     call-saved
	arg 2		%ecx	     call-clobbered
	arg 3		%edx	     call-clobbered
	arg 4		%esi	     call-saved
	arg 5		%edi	     call-saved

   The stack layout upon entering the function is:

	20(%esp)	Arg# 5
	16(%esp)	Arg# 4
	12(%esp)	Arg# 3
	 8(%esp)	Arg# 2
	 4(%esp)	Arg# 1
	  (%esp)	Return address

   (Of course a function with say 3 arguments does not have entries for
   arguments 4 and 5.)

   The following code tries hard to be optimal.  A general assuption
   (which is true accoriding to the data books I have) is that

	2 * xchg	is more expensive than	pushl + movl + popl

   Beside this a neat trick is used.  The calling conventions for Linux
   tell that among the registers used for parameters %ecx and %edx need
   not be saved.  Beside this we may clobber this registers even when
   they are not used for parameter passing.

   As a result one can see below that we save the content of the %ebx
   register in the %edx register when we have less than 3 arguments
   (2 * movl is less expensive than pushl + popl).

   Second unlike for the other registers we don't save the content of
   %ecx and %edx when we have than 1 and 2 registers resp.  */

#undef	DO_CALL
#define DO_CALL(args)					      		      \
    DOARGS_##args							      \
    int $0x80;								      \
    UNDOARGS_##args

#define	DOARGS_0	/* No arguments to frob.  */
#define	UNDOARGS_0	/* No arguments to unfrob.  */
#define	_DOARGS_0(n)	/* No arguments to frob.  */
#define	_UNDOARGS_0	/* No arguments to unfrob.  */

#define	DOARGS_1	movl %ebx, %edx; movl 4(%esp), %ebx; DOARGS_0
#define	UNDOARGS_1	UNDOARGS_0; movl %edx, %ebx
#define	_DOARGS_1(n)	pushl %ebx; movl n+4(%esp), %ebx; _DOARGS_0 (n)
#define	_UNDOARGS_1	_UNDOARGS_0; popl %ebx

#define	DOARGS_2	movl 8(%esp), %ecx; DOARGS_1
#define	UNDOARGS_2	UNDOARGS_1
#define	_DOARGS_2(n)	movl n(%esp), %ecx; _DOARGS_1 (n-4)
#define	_UNDOARGS_2	_UNDOARGS_1

#define DOARGS_3	_DOARGS_3 (12)
#define UNDOARGS_3	_UNDOARGS_3
#define _DOARGS_3(n)	movl n(%esp), %edx; _DOARGS_2 (n-4)
#define _UNDOARGS_3	_UNDOARGS_2

#define DOARGS_4	_DOARGS_4 (16)
#define UNDOARGS_4	_UNDOARGS_4
#define _DOARGS_4(n)	pushl %esi; movl n+4(%esp), %esi; _DOARGS_3 (n)
#define _UNDOARGS_4	_UNDOARGS_3; popl %esi

#define DOARGS_5	_DOARGS_5 (20)
#define UNDOARGS_5	_UNDOARGS_5
#define _DOARGS_5(n)	pushl %edi; movl n+4(%esp), %edi; _DOARGS_4 (n)
#define _UNDOARGS_5	_UNDOARGS_4; popl %edi


#endif	/* ASSEMBLER */
