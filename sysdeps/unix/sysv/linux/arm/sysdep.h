/* Copyright (C) 1992, 93, 95-99, 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper, <drepper@gnu.ai.mit.edu>, August 1995.
   ARM changes by Philip Blundell, <pjb27@cam.ac.uk>, May 1997.

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
  .type syscall_error,%function;					      \
  ENTRY (name);								      \
    DO_CALL (args, syscall_name);					      \
    cmn r0, $4096;

#define PSEUDO_RET							      \
    RETINSTR(movcc, pc, lr);						      \
    b PLTJMP(__syscall_error)
#undef ret
#define ret PSEUDO_RET

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
	arg 6		r5
	arg 7		r6

   The compiler is going to form a call by coming here, through PSEUDO, with
   arguments
   	syscall number	in the DO_CALL macro
   	arg 1		r0
   	arg 2		r1
   	arg 3		r2
   	arg 4		r3
   	arg 5		[sp]
	arg 6		[sp+4]
	arg 7		[sp+8]

   We need to shuffle values between R4..R6 and the stack so that the
   caller's v1..v3 and stack frame are not corrupted, and the kernel
   sees the right arguments.

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
#define DOARGS_5 str r4, [sp, $-4]!; ldr r4, [sp, $4];
#define DOARGS_6 mov ip, sp; stmfd sp!, {r4, r5}; ldmia ip, {r4, r5};
#define DOARGS_7 mov ip, sp; stmfd sp!, {r4, r5, r6}; ldmia ip, {r4, r5, r6};

#define UNDOARGS_0 /* nothing */
#define UNDOARGS_1 /* nothing */
#define UNDOARGS_2 /* nothing */
#define UNDOARGS_3 /* nothing */
#define UNDOARGS_4 /* nothing */
#define UNDOARGS_5 ldr r4, [sp], $4;
#define UNDOARGS_6 ldmfd sp!, {r4, r5};
#define UNDOARGS_7 ldmfd sp!, {r4, r5, r6};

#else /* not __ASSEMBLER__ */

/* Define a macro which expands into the inline wrapper code for a system
   call.  */
#undef INLINE_SYSCALL
#define INLINE_SYSCALL(name, nr, args...)			\
  ({ unsigned int _sys_result;					\
     {								\
       register int _a1 asm ("a1");				\
       LOAD_ARGS_##nr (args)					\
       asm volatile ("swi	%1	@ syscall " #name	\
		     : "=r" (_a1)				\
		     : "i" (SYS_ify(name)) ASM_ARGS_##nr	\
		     : "a1", "memory");				\
       _sys_result = _a1;					\
     }								\
     if (_sys_result >= (unsigned int) -4095)			\
       {							\
	 __set_errno (-_sys_result);				\
	 _sys_result = (unsigned int) -1;			\
       }							\
     (int) _sys_result; })

#define LOAD_ARGS_0()
#define ASM_ARGS_0
#define LOAD_ARGS_1(a1)				\
  _a1 = (int) (a1);				\
  LOAD_ARGS_0 ()
#define ASM_ARGS_1	ASM_ARGS_0, "r" (_a1)
#define LOAD_ARGS_2(a1, a2)			\
  register int _a2 asm ("a2") = (int) (a2);	\
  LOAD_ARGS_1 (a1)
#define ASM_ARGS_2	ASM_ARGS_1, "r" (_a2)
#define LOAD_ARGS_3(a1, a2, a3)			\
  register int _a3 asm ("a3") = (int) (a3);	\
  LOAD_ARGS_2 (a1, a2)
#define ASM_ARGS_3	ASM_ARGS_2, "r" (_a3)
#define LOAD_ARGS_4(a1, a2, a3, a4)		\
  register int _a4 asm ("a4") = (int) (a4);	\
  LOAD_ARGS_3 (a1, a2, a3)
#define ASM_ARGS_4	ASM_ARGS_3, "r" (_a4)
#define LOAD_ARGS_5(a1, a2, a3, a4, a5)		\
  register int _v1 asm ("v1") = (int) (a5);	\
  LOAD_ARGS_4 (a1, a2, a3, a4)
#define ASM_ARGS_5	ASM_ARGS_4, "r" (_v1)
#define LOAD_ARGS_6(a1, a2, a3, a4, a5, a6)	\
  register int _v2 asm ("v2") = (int) (a6);	\
  LOAD_ARGS_5 (a1, a2, a3, a4, a5)
#define ASM_ARGS_6	ASM_ARGS_5, "r" (_v2)
#define LOAD_ARGS_7(a1, a2, a3, a4, a5, a6, a7)	\
  register int _v3 asm ("v3") = (int) (a7);	\
  LOAD_ARGS_6 (a1, a2, a3, a4, a5, a6)
#define ASM_ARGS_7	ASM_ARGS_6, "r" (_v3)

#endif	/* __ASSEMBLER__ */

#endif /* linux/arm/sysdep.h */
