/* Copyright (C) 1999, 2000, 2002, 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Written by Jes Sorensen, <Jes.Sorensen@cern.ch>, April 1999.
   Based on code originally written by David Mosberger-Tang

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

#ifndef _LINUX_IA64_SYSDEP_H
#define _LINUX_IA64_SYSDEP_H 1

#include <sysdeps/unix/sysdep.h>
#include <sysdeps/ia64/sysdep.h>

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

/* This is a kludge to make syscalls.list find these under the names
   pread and pwrite, since some kernel headers define those names
   and some define the *64 names for the same system calls.  */
#if !defined __NR_pread && defined __NR_pread64
# define __NR_pread __NR_pread64
#endif
#if !defined __NR_pwrite && defined __NR_pwrite64
# define __NR_pwrite __NR_pwrite64
#endif

#ifdef __ASSEMBLER__

#undef CALL_MCOUNT
#ifdef PROF
# define CALL_MCOUNT							\
	.data;								\
1:	data8 0;	/* XXX fixme: use .xdata8 once labels work */	\
	.previous;							\
	.prologue;							\
	.save ar.pfs, r40;						\
	alloc out0 = ar.pfs, 8, 0, 4, 0;				\
	mov out1 = gp;							\
	.save rp, out2;							\
	mov out2 = rp;							\
	.body;								\
	;;								\
	addl out3 = @ltoff(1b), gp;					\
	br.call.sptk.many rp = _mcount					\
	;;
#else
# define CALL_MCOUNT	/* Do nothing. */
#endif

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
#define SYSCALL_ERROR_LABEL __syscall_error

#undef PSEUDO
#define	PSEUDO(name, syscall_name, args)	\
  ENTRY(name)					\
    DO_CALL (SYS_ify(syscall_name));		\
	cmp.eq p6,p0=-1,r10;			\
(p6)	br.cond.spnt.few __syscall_error;

#define DO_CALL(num)				\
	mov r15=num;				\
	break __BREAK_SYSCALL;

#undef PSEUDO_END
#define PSEUDO_END(name)	.endp C_SYMBOL_NAME(name);

#undef END
#define END(name)						\
	.size	C_SYMBOL_NAME(name), . - C_SYMBOL_NAME(name) ;	\
	.endp	C_SYMBOL_NAME(name)

#define ret			br.ret.sptk.few b0

#else /* not __ASSEMBLER__ */

/* On IA-64 we have stacked registers for passing arguments.  The
   "out" registers end up being the called function's "in"
   registers.

   Also, since we have plenty of registers we have two return values
   from a syscall.  r10 is set to -1 on error, whilst r8 contains the
   (non-negative) errno on error or the return value on success.
 */
#undef INLINE_SYSCALL
#define INLINE_SYSCALL(name, nr, args...)			\
  ({								\
    register long _r8 asm ("r8");				\
    register long _r10 asm ("r10");				\
    register long _r15 asm ("r15") = __NR_##name;		\
    long _retval;						\
    LOAD_ARGS_##nr (args);					\
    __asm __volatile ("break %3;;\n\t"				\
                      : "=r" (_r8), "=r" (_r10), "=r" (_r15)	\
                      : "i" (__BREAK_SYSCALL), "2" (_r15)	\
			ASM_ARGS_##nr				\
                      : "memory" ASM_CLOBBERS_##nr);		\
    _retval = _r8;						\
    if (_r10 == -1)						\
      {								\
        __set_errno (_retval);					\
        _retval = -1;						\
      }								\
    _retval; })

#undef INTERNAL_SYSCALL_DECL
#define INTERNAL_SYSCALL_DECL(err) long int err

#undef INTERNAL_SYSCALL
#define INTERNAL_SYSCALL(name, err, nr, args...)		\
  ({								\
    register long _r8 asm ("r8");				\
    register long _r10 asm ("r10");				\
    register long _r15 asm ("r15") = __NR_##name;		\
    long _retval;						\
    LOAD_ARGS_##nr (args);					\
    __asm __volatile ("break %3;;\n\t"				\
                      : "=r" (_r8), "=r" (_r10), "=r" (_r15)	\
                      : "i" (__BREAK_SYSCALL), "2" (_r15)	\
			ASM_ARGS_##nr				\
                      : "memory" ASM_CLOBBERS_##nr);		\
    _retval = _r8;						\
    err = _r10;							\
    _retval; })

#undef INTERNAL_SYSCALL_ERROR_P
#define INTERNAL_SYSCALL_ERROR_P(val, err)	(err == -1)

#undef INTERNAL_SYSCALL_ERRNO
#define INTERNAL_SYSCALL_ERRNO(val, err)	(val)

#define LOAD_ARGS_0()   do { } while (0)
#define LOAD_ARGS_1(out0)				\
  register long _out0 asm ("out0") = (long) (out0);	\
  LOAD_ARGS_0 ()
#define LOAD_ARGS_2(out0, out1)				\
  register long _out1 asm ("out1") = (long) (out1);	\
  LOAD_ARGS_1 (out0)
#define LOAD_ARGS_3(out0, out1, out2)			\
  register long _out2 asm ("out2") = (long) (out2);	\
  LOAD_ARGS_2 (out0, out1)
#define LOAD_ARGS_4(out0, out1, out2, out3)		\
  register long _out3 asm ("out3") = (long) (out3);	\
  LOAD_ARGS_3 (out0, out1, out2)
#define LOAD_ARGS_5(out0, out1, out2, out3, out4)	\
  register long _out4 asm ("out4") = (long) (out4);	\
  LOAD_ARGS_4 (out0, out1, out2, out3)

#define ASM_ARGS_0
#define ASM_ARGS_1      ASM_ARGS_0, "r" (_out0)
#define ASM_ARGS_2      ASM_ARGS_1, "r" (_out1)
#define ASM_ARGS_3      ASM_ARGS_2, "r" (_out2)
#define ASM_ARGS_4      ASM_ARGS_3, "r" (_out3)
#define ASM_ARGS_5      ASM_ARGS_4, "r" (_out4)

#define ASM_CLOBBERS_0	ASM_CLOBBERS_1, "out0"
#define ASM_CLOBBERS_1	ASM_CLOBBERS_2, "out1"
#define ASM_CLOBBERS_2	ASM_CLOBBERS_3, "out2"
#define ASM_CLOBBERS_3	ASM_CLOBBERS_4, "out3"
#define ASM_CLOBBERS_4	ASM_CLOBBERS_5, "out4"
#define ASM_CLOBBERS_5	, "out5", "out6", "out7",			\
  /* Non-stacked integer registers, minus r8, r10, r15.  */		\
  "r2", "r3", "r9", "r11", "r12", "r13", "r14", "r16", "r17", "r18",	\
  "r19", "r20", "r21", "r22", "r23", "r24", "r25", "r26", "r27",	\
  "r28", "r29", "r30", "r31",						\
  /* Predicate registers.  */						\
  "p6", "p7", "p8", "p9", "p10", "p11", "p12", "p13", "p14", "p15",	\
  /* Non-rotating fp registers.  */					\
  "f6", "f7", "f8", "f9", "f10", "f11", "f12", "f13", "f14", "f15",	\
  /* Branch registers.  */						\
  "b6", "b7"

#endif /* not __ASSEMBLER__ */

#endif /* linux/ia64/sysdep.h */
