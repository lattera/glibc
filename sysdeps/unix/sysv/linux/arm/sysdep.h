/* Copyright (C) 1992, 93, 95, 96, 97, 98 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper, <drepper@gnu.ai.mit.edu>, August 1995.
   ARM changes by Philip Blundell, <pjb27@cam.ac.uk>, May 1997.

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

#ifndef _LINUX_ARM_SYSDEP_H
#define _LINUX_ARM_SYSDEP_H 1

/* There is some commonality.  */
#include <sysdeps/unix/arm/sysdep.h>

/* For Linux we can use the system call table in the header file
	/usr/include/asm/unistd.h
   of the kernel.  But these symbols do not follow the SYS_* syntax
   so we have to redefine the `SYS_ify' macro here.  */
#undef SYS_ify
#define SWI_BASE  (0x900000)
#define SYS_ify(syscall_name)	(__NR_##syscall_name)


#ifdef __ASSEMBLER__

/* Linux uses a negative return value to indicate syscall errors,
   unlike most Unices, which use the condition codes' carry flag.

   Since version 2.1 the return value of a system call might be
   negative even if the call succeeded.  E.g., the `lseek' system call
   might return a large offset.  Therefore we must not anymore test
   for < 0, but test for a real error by making sure the value in R0
   is a real error number.  Linus said he will make sure the no syscall
   returns a value in -1 .. -4095 as a valid result so we can savely
   test with -4095.  */

#undef	PSEUDO
#define	PSEUDO(name, syscall_name, args)				      \
  .text;								      \
  .type syscall_error,%function	;					      \
  ENTRY (name)								      \
    DO_CALL (args, syscall_name);					      \
    cmn r0, $4096;							      \
    bhs PLTJMP(C_SYMBOL_NAME(__syscall_error));

#undef	PSEUDO_END
#define	PSEUDO_END(name)						      \
  SYSCALL_ERROR_HANDLER							      \
  END (name)

#define SYSCALL_ERROR_HANDLER	/* Nothing here; code in sysdep.S is used.  */

/* Linux takes system call args in registers:
	syscall number	in the SWI instruction
	arg 1		r0
	arg 2		r1
	arg 3		r2
	arg 4		r3
	arg 5		r4	(this is different from the APCS convention)

   The compiler is going to form a call by coming here, through PSEUDO, with
   arguments
   	syscall number	in the DO_CALL macro
   	arg 1		r0
   	arg 2		r1
   	arg 3		r2
   	arg 4		r3
   	arg 5		[sp]

   We need to shuffle values between R4 and the stack so that the caller's
   R4 is not corrupted, and the kernel sees the right argument there.

*/

#undef	DO_CALL
#define DO_CALL(args, syscall_name)		\
    DOARGS_##args				\
    swi SYS_ify (syscall_name); 		\
    UNDOARGS_##args

#define DOARGS_0 /* nothing */
#define DOARGS_1 /* nothing */
#define DOARGS_2 /* nothing */
#define DOARGS_3 /* nothing */
#define DOARGS_4 /* nothing */
#define DOARGS_5 ldr ip, [sp]; str r4, [sp]; mov r4, ip;

#define UNDOARGS_0 /* nothing */
#define UNDOARGS_1 /* nothing */
#define UNDOARGS_2 /* nothing */
#define UNDOARGS_3 /* nothing */
#define UNDOARGS_4 /* nothing */
#define UNDOARGS_5 ldr r4, [sp];

#endif	/* __ASSEMBLER__ */

#endif /* linux/arm/sysdep.h */
