/* Assembler macros for PA-RISC.
   Copyright (C) 1999, 2001 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper, <drepper@cygnus.com>, August 1999.
   Linux/PA-RISC changes by Philipp Rumpf, <prumpf@tux.org>, March 2000.

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

#include <asm/unistd.h>
#include <sysdeps/generic/sysdep.h>
#include <sys/syscall.h>
#include "config.h"

#ifndef ASM_LINE_SEP
#define ASM_LINE_SEP ;
#endif

#undef SYS_ify
#define SYS_ify(syscall_name)	(__NR_##syscall_name)


#ifdef __ASSEMBLER__

/* Syntactic details of assembler.  */

#define ALIGNARG(log2) log2

/* For Linux we can use the system call table in the header file
	/usr/include/asm/unistd.h
   of the kernel.  But these symbols do not follow the SYS_* syntax
   so we have to redefine the `SYS_ify' macro here.  */
#undef SYS_ify
#define SYS_ify(syscall_name)	__NR_##syscall_name

/* ELF-like local names start with `.L'.  */
#undef L
#define L(name)	.L##name

/* Linux uses a negative return value to indicate syscall errors,
   unlike most Unices, which use the condition codes' carry flag.

   Since version 2.1 the return value of a system call might be
   negative even if the call succeeded.  E.g., the `lseek' system call
   might return a large offset.  Therefore we must not anymore test
   for < 0, but test for a real error by making sure the value in %eax
   is a real error number.  Linus said he will make sure the no syscall
   returns a value in -1 .. -4095 as a valid result so we can safely
   test with -4095.  */

/* We don't want the label for the error handle to be global when we define
   it here.  */
#ifdef PIC
# define SYSCALL_ERROR_LABEL 0f
#else
# define SYSCALL_ERROR_LABEL syscall_error
#endif

/* Define an entry point visible from C.

   There is currently a bug in gdb which prevents us from specifying
   incomplete stabs information.  Fake some entries here which specify
   the current source file.  */
#define	ENTRY(name)						\
	.text					ASM_LINE_SEP	\
	.export C_SYMBOL_NAME(name)		ASM_LINE_SEP	\
	.type	C_SYMBOL_NAME(name),@function	ASM_LINE_SEP	\
	C_LABEL(name)						\
	CALL_MCOUNT

#define ret \
	bv 0(2)					ASM_LINE_SEP	\
	nop

#undef	END
#define END(name)						\
1:	.size	C_SYMBOL_NAME(name),1b-C_SYMBOL_NAME(name)

/* If compiled for profiling, call `mcount' at the start of each function.  */
/* No, don't bother.  gcc will put the call in for us.  */
#define CALL_MCOUNT		/* Do nothing.  */

/* syscall wrappers consist of
	#include <sysdep.h>
	PSEUDO(...)
	ret
	PSEUDO_END(...)

   which means
	ENTRY(name)
	DO_CALL(...)
	nop
	bv 0(2)
	nop
*/

#define	PSEUDO(name, syscall_name, args)				      \
  ENTRY (name)								      \
  DO_CALL(args, syscall_name)					ASM_LINE_SEP  \
  nop

#undef	PSEUDO_END
#define	PSEUDO_END(name)						      \
  END (name)

#define JUMPTARGET(name)	name
#define SYSCALL_PIC_SETUP	/* Nothing.  */

/* Linux takes system call arguments in registers:
	syscall number	gr20
	arg 1		gr26
	arg 2		gr25
	arg 3		gr24
	arg 4		gr23
	arg 5		gr22
	arg 6		gr21

   The compiler calls us by the C convention:
	syscall number	in the DO_CALL macro
	arg 1		gr26
	arg 2		gr25
	arg 3		gr24
	arg 4		gr23
	arg 5		-52(gr30)
	arg 6		-56(gr30)

   gr22 and gr21 are caller-saves, so we can just load the arguments
   there and generally be happy. */

/* the cmpb...no_error code below inside DO_CALL
 * is intended to mimic the if (__sys_res...)
 * code inside INLINE_SYSCALL
 */

#undef	DO_CALL
#define DO_CALL(args, syscall_name)				\
	DOARGS_##args						\
	ble  0x100(%sr2,%r0)			ASM_LINE_SEP	\
	ldi SYS_ify (syscall_name), %r20	ASM_LINE_SEP	\
	ldi -0x1000,%r1				ASM_LINE_SEP	\
	cmpb,>>=,n %r1,%ret0,0f			ASM_LINE_SEP	\
	stw %rp, -20(%sr0,%r30)			ASM_LINE_SEP	\
	stw %ret0, -24(%sr0,%r30)		ASM_LINE_SEP	\
	.import __errno_location,code		ASM_LINE_SEP	\
	bl __errno_location,%rp			ASM_LINE_SEP	\
	ldo 64(%r30), %r30			ASM_LINE_SEP	\
	ldo -64(%r30), %r30			ASM_LINE_SEP	\
	ldw -24(%r30), %r26			ASM_LINE_SEP	\
	sub %r0, %r26, %r26			ASM_LINE_SEP	\
	stw %r26, 0(%sr0,%ret0)			ASM_LINE_SEP	\
	ldo -1(%r0), %ret0			ASM_LINE_SEP	\
	ldw -20(%r30), %rp			ASM_LINE_SEP	\
0:						ASM_LINE_SEP	\
	UNDOARGS_##args

#define DOARGS_0 /* nothing */
#define DOARGS_1 /* nothing */
#define DOARGS_2 /* nothing */
#define DOARGS_3 /* nothing */
#define DOARGS_4 /* nothing */
#define DOARGS_5 ldw -52(%r30), %r22		ASM_LINE_SEP
#define DOARGS_6 ldw -52(%r30), %r22		ASM_LINE_SEP	\
		 ldw -56(%r30), %r21		ASM_LINE_SEP


#define UNDOARGS_0 /* nothing */
#define UNDOARGS_1 /* nothing */
#define UNDOARGS_2 /* nothing */
#define UNDOARGS_3 /* nothing */
#define UNDOARGS_4 /* nothing */
#define UNDOARGS_5 /* nothing */
#define UNDOARGS_6 /* nothing */

#else

#undef INLINE_SYSCALL
#define INLINE_SYSCALL(name, nr, args...)	({		\
	long __sys_res;						\
	{							\
		register unsigned long __res asm("r28");	\
		LOAD_ARGS_##nr(args)				\
		asm volatile(					\
			"ble  0x100(%%sr2, %%r0)\n\t"		\
			" ldi %1, %%r20"			\
			: "=r" (__res)				\
			: "i" (SYS_ify(name)) ASM_ARGS_##nr	\
			 );					\
		__sys_res = __res;				\
	}							\
	if ((unsigned long)__sys_res >= (unsigned long)-4095) {	\
		__set_errno(-__sys_res);			\
		__sys_res = -1;					\
	}							\
	__sys_res;						\
})

#define LOAD_ARGS_0()
#define LOAD_ARGS_1(r26)					\
	register unsigned long __r26 __asm__("r26") = (unsigned long)r26;	\
	LOAD_ARGS_0()
#define LOAD_ARGS_2(r26,r25)					\
	register unsigned long __r25 __asm__("r25") = (unsigned long)r25;	\
	LOAD_ARGS_1(r26)
#define LOAD_ARGS_3(r26,r25,r24)				\
	register unsigned long __r24 __asm__("r24") = (unsigned long)r24;	\
	LOAD_ARGS_2(r26,r25)
#define LOAD_ARGS_4(r26,r25,r24,r23)				\
	register unsigned long __r23 __asm__("r23") = (unsigned long)r23;	\
	LOAD_ARGS_3(r26,r25,r24)
#define LOAD_ARGS_5(r26,r25,r24,r23,r22)			\
	register unsigned long __r22 __asm__("r22") = (unsigned long)r22;	\
	LOAD_ARGS_4(r26,r25,r24,r23)
#define LOAD_ARGS_6(r26,r25,r24,r23,r22,r21)			\
	register unsigned long __r21 __asm__("r21") = (unsigned long)r21;	\
	LOAD_ARGS_5(r26,r25,r24,r23,r22)

#define ASM_ARGS_0
#define ASM_ARGS_1 , "r" (__r26)
#define ASM_ARGS_2 , "r" (__r26), "r" (__r25)
#define ASM_ARGS_3 , "r" (__r26), "r" (__r25), "r" (__r24)
#define ASM_ARGS_4 , "r" (__r26), "r" (__r25), "r" (__r24), "r" (__r23)
#define ASM_ARGS_5 , "r" (__r26), "r" (__r25), "r" (__r24), "r" (__r23), "r" (__r22)
#define ASM_ARGS_6 , "r" (__r26), "r" (__r25), "r" (__r24), "r" (__r23), "r" (__r22), "r" (__r21)

#endif	/* __ASSEMBLER__ */
