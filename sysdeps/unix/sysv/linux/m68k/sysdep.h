/* Copyright (C) 1996, 1997, 1998, 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Written by Andreas Schwab, <schwab@issan.informatik.uni-dortmund.de>,
   December 1995.

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

#include <sysdeps/unix/sysdep.h>
#include <sysdeps/m68k/sysdep.h>

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

#ifdef __ASSEMBLER__

/* Linux uses a negative return value to indicate syscall errors, unlike
   most Unices, which use the condition codes' carry flag.

   Since version 2.1 the return value of a system call might be negative
   even if the call succeeded.  E.g., the `lseek' system call might return
   a large offset.  Therefore we must not anymore test for < 0, but test
   for a real error by making sure the value in %d0 is a real error
   number.  Linus said he will make sure the no syscall returns a value
   in -1 .. -4095 as a valid result so we can savely test with -4095.  */

/* We don't want the label for the error handler to be visible in the symbol
   table when we define it here.  */
#ifdef PIC
#define SYSCALL_ERROR_LABEL .Lsyscall_error
#else
#define SYSCALL_ERROR_LABEL __syscall_error
#endif

#undef PSEUDO
#define	PSEUDO(name, syscall_name, args)				      \
  .text;								      \
  ENTRY (name)								      \
    DO_CALL (syscall_name, args);					      \
    cmp.l &-4095, %d0;							      \
    jcc SYSCALL_ERROR_LABEL

#undef PSEUDO_END
#define PSEUDO_END(name)						      \
  SYSCALL_ERROR_HANDLER;						      \
  END (name)

#ifdef PIC
/* Store (- %d0) into errno through the GOT.  */
#ifdef _LIBC_REENTRANT
#define SYSCALL_ERROR_HANDLER						      \
SYSCALL_ERROR_LABEL:							      \
    neg.l %d0;								      \
    move.l %d0, -(%sp);							      \
    jbsr __errno_location@PLTPC;					      \
    move.l (%sp)+, (%a0);						      \
    move.l &-1, %d0;							      \
    /* Copy return value to %a0 for syscalls that are declared to return      \
       a pointer (e.g., mmap).  */					      \
    move.l %d0, %a0;							      \
    rts;
#else /* !_LIBC_REENTRANT */
#define SYSCALL_ERROR_HANDLER						      \
SYSCALL_ERROR_LABEL:							      \
    move.l (errno@GOTPC, %pc), %a0;					      \
    neg.l %d0;								      \
    move.l %d0, (%a0);							      \
    move.l &-1, %d0;							      \
    /* Copy return value to %a0 for syscalls that are declared to return      \
       a pointer (e.g., mmap).  */					      \
    move.l %d0, %a0;							      \
    rts;
#endif /* _LIBC_REENTRANT */
#else
#define SYSCALL_ERROR_HANDLER	/* Nothing here; code in sysdep.S is used.  */
#endif /* PIC */

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

#define DO_CALL(syscall_name, args)			      		      \
    move.l &SYS_ify(syscall_name), %d0;					      \
    DOARGS_##args							      \
    trap &0;								      \
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

#else /* not __ASSEMBLER__ */

/* Define a macro which expands into the inline wrapper code for a system
   call.  */
#undef INLINE_SYSCALL
#define INLINE_SYSCALL(name, nr, args...)		\
  ({ unsigned int _sys_result;				\
     {							\
       LOAD_ARGS_##nr (args)				\
       register int _d0 asm ("%d0") = __NR_##name;	\
       asm volatile ("trap #0"				\
		     : "=d" (_d0)			\
		     : "0" (_d0) ASM_ARGS_##nr		\
		     : "memory");			\
       _sys_result = _d0;				\
     }							\
     if (_sys_result >= (unsigned int) -4095)		\
       {						\
	 __set_errno (-_sys_result);			\
	 _sys_result = (unsigned int) -1;		\
       }						\
     (int) _sys_result; })

#define LOAD_ARGS_0()
#define ASM_ARGS_0
#define LOAD_ARGS_1(a1)				\
  register int _d1 asm ("d1") = (int) (a1);	\
  LOAD_ARGS_0 ()
#define ASM_ARGS_1	ASM_ARGS_0, "d" (_d1)
#define LOAD_ARGS_2(a1, a2)			\
  register int _d2 asm ("d2") = (int) (a2);	\
  LOAD_ARGS_1 (a1)
#define ASM_ARGS_2	ASM_ARGS_1, "d" (_d2)
#define LOAD_ARGS_3(a1, a2, a3)			\
  register int _d3 asm ("d3") = (int) (a3);	\
  LOAD_ARGS_2 (a1, a2)
#define ASM_ARGS_3	ASM_ARGS_2, "d" (_d3)
#define LOAD_ARGS_4(a1, a2, a3, a4)		\
  register int _d4 asm ("d4") = (int) (a4);	\
  LOAD_ARGS_3 (a1, a2, a3)
#define ASM_ARGS_4	ASM_ARGS_3, "d" (_d4)
#define LOAD_ARGS_5(a1, a2, a3, a4, a5)		\
  register int _d5 asm ("d5") = (int) (a5);	\
  LOAD_ARGS_4 (a1, a2, a3, a4)
#define ASM_ARGS_5	ASM_ARGS_4, "d" (_d5)

#endif /* not __ASSEMBLER__ */
