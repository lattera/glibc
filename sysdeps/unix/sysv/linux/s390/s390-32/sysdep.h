/* Copyright (C) 2000, 2001 Free Software Foundation, Inc.
   Contributed by Martin Schwidefsky (schwidefsky@de.ibm.com).
   This file is part of the GNU C Library.

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

#ifndef _LINUX_S390_SYSDEP_H
#define _LINUX_S390_SYSDEP_H

#include <sysdeps/s390/s390-32/sysdep.h>
#include <sysdeps/unix/sysdep.h>

/* For Linux we can use the system call table in the header file
	/usr/include/asm/unistd.h
   of the kernel.  But these symbols do not follow the SYS_* syntax
   so we have to redefine the `SYS_ify' macro here.  */
/* in newer 2.1 kernels __NR_syscall is missing so we define it here */
#define __NR_syscall 0

#undef SYS_ify
#define SYS_ify(syscall_name)	__NR_##syscall_name

/* ELF-like local names start with `.L'.  */
#undef L
#define L(name)	.L##name

#ifdef __ASSEMBLER__

/* Linux uses a negative return value to indicate syscall errors, unlike
   most Unices, which use the condition codes' carry flag.

   Since version 2.1 the return value of a system call might be negative
   even if the call succeeded.  E.g., the `lseek' system call might return
   a large offset.  Therefore we must not anymore test for < 0, but test
   for a real error by making sure the value in gpr2 is a real error
   number.  Linus said he will make sure the no syscall returns a value
   in -1 .. -4095 as a valid result so we can savely test with -4095.  */

#define SYSCALL_ERROR_LABEL 0f

#undef PSEUDO
#define	PSEUDO(name, syscall_name, args)				      \
  .text;                                                                      \
  ENTRY (name)							              \
    DO_CALL (args, syscall_name);                                             \
    lhi  %r4,-4095 ;                                                          \
    clr  %r2,%r4 ;		                                              \
    jnl  SYSCALL_ERROR_LABEL ;                                                \
  L(pseudo_end):

#undef PSEUDO_END
#define PSEUDO_END(name)						      \
  SYSCALL_ERROR_HANDLER;						      \
  END (name)

#ifndef _LIBC_REENTRANT
#ifndef PIC
#define SYSCALL_ERROR_HANDLER                                                 \
0:  lcr     %r2,%r2 ;                                                         \
    basr    %r1,0 ;                                                           \
1:  l       %r1,2f-1b(%r1)                                                    \
    st      %r2,0(%r1)                                                        \
    lhi     %r2,-1                                                            \
    br      %r14                                                              \
2:  .long   errno
#else
#define SYSCALL_ERROR_HANDLER						      \
0:  basr    %r1,0 ;                                                           \
1:  al      %r1,2f-1b(%r1) ;                                                  \
    l       %r1,errno@GOT12(%r1) ;                                            \
    lcr     %r2,%r2 ;							      \
    st      %r2,0(%r1) ;						      \
    lhi     %r2,-1 ;                                                          \
    br      %r14 ;                                                            \
2:  .long   _GLOBAL_OFFSET_TABLE_-1b
#endif /* PIC */
#else
#define SYSCALL_ERROR_HANDLER                                                 \
0:  basr    %r1,0 ;                                                           \
1:  al      %r1,2f-1b(%r1) ;                                                  \
    br      %r1 ;                                                             \
2:  .long   __syscall_error@PLT-1b
#endif /* _LIBC_REENTRANT */

/* Linux takes system call arguments in registers:

	syscall number	1	     call-clobbered
	arg 1		2	     call-clobbered
	arg 2		3	     call-clobbered
	arg 3		4	     call-clobbered
	arg 4		5	     call-clobbered
	arg 5		6	     call-saved

   (Of course a function with say 3 arguments does not have entries for
   arguments 4 and 5.)
   S390 does not need to do ANY stack operations to get its parameters
   right.
 */

#define DO_CALL(args, syscall)						      \
    svc     SYS_ify (syscall)

#define ret                                                                   \
    br      14

#endif /* __ASSEMBLER__ */

#undef INLINE_SYSCALL
#define INLINE_SYSCALL(name, nr, args...)                                     \
  ({                                                                          \
    DECLARGS_##nr(args)                                                       \
    int err;                                                                  \
    asm volatile (                                                            \
    LOADARGS_##nr                                                             \
    "svc    %b1\n\t"                                                          \
    "lr     %0,%%r2\n\t"                                                      \
    : "=d" (err)                                                              \
    : "I" (__NR_##name) ASMFMT_##nr                                           \
    : "memory", "cc", "2", "3", "4", "5", "6");                               \
    if (err >= 0xfffff001)                                                    \
     {                                                                        \
       __set_errno(-err);                                                     \
       err = 0xffffffff;                                                      \
     }                                                                        \
    (int) err; })

#define DECLARGS_0()
#define DECLARGS_1(arg1) \
	unsigned int gpr2 = (unsigned int) (arg1);
#define DECLARGS_2(arg1, arg2) \
	DECLARGS_1(arg1) \
	unsigned int gpr3 = (unsigned int) (arg2);
#define DECLARGS_3(arg1, arg2, arg3) \
	DECLARGS_2(arg1, arg2) \
	unsigned int gpr4 = (unsigned int) (arg3);
#define DECLARGS_4(arg1, arg2, arg3, arg4) \
	DECLARGS_3(arg1, arg2, arg3) \
	unsigned int gpr5 = (unsigned int) (arg4);
#define DECLARGS_5(arg1, arg2, arg3, arg4, arg5) \
	DECLARGS_4(arg1, arg2, arg3, arg4) \
	unsigned int gpr6 = (unsigned int) (arg5);

#define LOADARGS_0
#define LOADARGS_1            "L     2,%2\n\t"
#define LOADARGS_2 LOADARGS_1 "L     3,%3\n\t"
#define LOADARGS_3 LOADARGS_2 "L     4,%4\n\t"
#define LOADARGS_4 LOADARGS_3 "L     5,%5\n\t"
#define LOADARGS_5 LOADARGS_4 "L     6,%6\n\t"

#define ASMFMT_0
#define ASMFMT_1 , "m" (gpr2)
#define ASMFMT_2 , "m" (gpr2), "m" (gpr3)
#define ASMFMT_3 , "m" (gpr2), "m" (gpr3), "m" (gpr4)
#define ASMFMT_4 , "m" (gpr2), "m" (gpr3), "m" (gpr4), "m" (gpr5)
#define ASMFMT_5 , "m" (gpr2), "m" (gpr3), "m" (gpr4), "m" (gpr5), "m" (gpr6)

#if 0
#undef INLINE_SYSCALL
#define INLINE_SYSCALL(name, nr, args...)                                     \
  ({                                                                          \
    DECLARGS_##nr(args)                                                       \
    asm volatile (                                                            \
    "svc    %b1\n\t"                                                          \
    : "+d" (gpr2)                                                             \
    : "I" (__NR_##name) ASMFMT_##nr : "memory", "cc");                        \
    if (gpr2 >= 0xfffff001)                                                   \
     {                                                                        \
       __set_errno(-gpr2);                                                    \
       gpr2 = 0xffffffff;                                                     \
     }                                                                        \
    (int) gpr2; })

#define DECLARGS_0() \
	register unsigned int gpr2 asm("2");
#define DECLARGS_1(arg1) \
	register unsigned int gpr2 asm("2") = (unsigned int) (arg1);
#define DECLARGS_2(arg1, arg2) \
	DECLARGS_1(arg1) \
	register unsigned int gpr3 asm("3") = (unsigned int) (arg2);
#define DECLARGS_3(arg1, arg2, arg3) \
	DECLARGS_2(arg1, arg2) \
	register unsigned int gpr4 asm("4") = (unsigned int) (arg3);
#define DECLARGS_4(arg1, arg2, arg3, arg4) \
	DECLARGS_3(arg1, arg2, arg3) \
	register unsigned int gpr5 asm("5") = (unsigned int) (arg4);
#define DECLARGS_5(arg1, arg2, arg3, arg4, arg5) \
	DECLARGS_4(arg1, arg2, arg3, arg4) \
	register unsigned int gpr6 asm("6") = (unsigned int) (arg5);

#define ASMFMT_0
#define ASMFMT_1
#define ASMFMT_2 , "d" (gpr3)
#define ASMFMT_3 , "d" (gpr3), "d" (gpr4)
#define ASMFMT_4 , "d" (gpr3), "d" (gpr4), "d" (gpr5)
#define ASMFMT_5 , "d" (gpr3), "d" (gpr4), "d" (gpr5), "d" (gpr6)

#endif /* 0 */

#endif /* _LINUX_S390_SYSDEP_H */
