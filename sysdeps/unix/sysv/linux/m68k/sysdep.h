/* Copyright (C) 1996 Free Software Foundation, Inc.
This file is part of the GNU C Library.
Written by Andreas Schwab, <schwab@issan.informatik.uni-dortmund.de>,
December 1995.

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

#include <sysdeps/unix/sysdep.h>

#ifdef ASSEMBLER

#define POUND #

/* Define an entry point visible from C.  */
#define	ENTRY(name)							      \
  .globl name;								      \
  .type name, @function;						      \
  .align 4;								      \
  name##:

/* Since C identifiers are not normally prefixed with an underscore
   on this system, the asm identifier `syscall_error' intrudes on the
   C name space.  Make sure we use an innocuous name.  */
#define	syscall_error	__syscall_error

/* Linux uses a negative return value to indicate syscall errors, unlike
   most Unices, which use the condition codes' carry flag.  */
#define	PSEUDO(name, syscall_name, args)				      \
  .text;								      \
  SYSCALL_ERROR_HANDLER							      \
  ENTRY (name)								      \
    DO_CALL (POUND SYS_ify (syscall_name), args);			      \
    tst.l %d0;								      \
    jmi syscall_error;

#ifdef PIC
/* Store (- %d0) into errno through the GOT.  */
#define SYSCALL_ERROR_HANDLER						      \
syscall_error:								      \
    move.l (errno@GOTPC.l, %pc), %a0;					      \
    neg.l %d0;								      \
    move.l %d0, (%a0);							      \
    move.l POUND -1, %d0;						      \
    /* Copy return value to %a0 for syscalls that are declared to return      \
       a pointer (e.g., mmap).  */					      \
    move.l %d0, %a0;							      \
    rts;
#else
#define SYSCALL_ERROR_HANDLER	/* Nothing here; code in sysdep.S is used.  */
#endif

/* Linux takes system call arguments in registers:

	syscall number	%d0	     call-clobbered
	arg 1		%d1	     call-clobbered
	arg 2		%d2	     call-saved
	arg 3		%d3	     call-saved
	arg 4		%d4	     call-saved
	arg 5		%d5	     call-saved

   The stack layout upon entering the function is:

	20(%sp)		Arg# 5
	16(%sp)		Arg# 4
	12(%sp)		Arg# 3
	 8(%sp)		Arg# 2
	 4(%sp)		Arg# 1
	  (%sp)		Return address

   (Of course a function with say 3 arguments does not have entries for
   arguments 4 and 5.)

   Separate move's are faster than movem, but need more space.  Since
   speed is more important, we don't use movem.  Since %a0 and %a1 are
   scratch registers, we can use them for saving as well.  */

#define DO_CALL(syscall, args)				      		      \
    move.l syscall, %d0;						      \
    DOARGS_##args							      \
    trap POUND 0;							      \
    UNDOARGS_##args

#define	DOARGS_0	/* No arguments to frob.  */
#define	UNDOARGS_0	/* No arguments to unfrob.  */
#define	_DOARGS_0(n)	/* No arguments to frob.  */

#define	DOARGS_1	_DOARGS_1 (4)
#define	_DOARGS_1(n)	move.l n(%sp), %d1; _DOARGS_0 (n)
#define	UNDOARGS_1	UNDOARGS_0

#define	DOARGS_2	_DOARGS_2 (8)
#define	_DOARGS_2(n)	move.l %d2, %a0; move.l n(%sp), %d2; _DOARGS_1 (n-4)
#define	UNDOARGS_2	UNDOARGS_1; move.l %a0, %d2

#define DOARGS_3	_DOARGS_3 (12)
#define _DOARGS_3(n)	move.l %d3, %a1; move.l n(%sp), %d3; _DOARGS_2 (n-4)
#define UNDOARGS_3	UNDOARGS_2; move.l %a1, %d3

#define DOARGS_4	_DOARGS_4 (16)
#define _DOARGS_4(n)	move.l %d4, -(%sp); move.l n+4(%sp), %d4; _DOARGS_3 (n)
#define UNDOARGS_4	UNDOARGS_3; move.l (%sp)+, %d4

#define DOARGS_5	_DOARGS_5 (20)
#define _DOARGS_5(n)	move.l %d5, -(%sp); move.l n+4(%sp), %d5; _DOARGS_4 (n)
#define UNDOARGS_5	UNDOARGS_4; move.l (%sp)+, %d5


#define	ret	rts
#if 0 /* Not used by Linux */
#define	r0	%d0
#define	r1	%d1
#define	MOVE(x,y)	movel x , y
#endif

#endif	/* ASSEMBLER */
