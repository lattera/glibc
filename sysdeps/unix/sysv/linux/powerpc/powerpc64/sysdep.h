/* Copyright (C) 1992, 1997, 1998, 1999, 2000, 2001, 2002
   Free Software Foundation, Inc.
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
		
/* Alan Modra <amodra@bigpond.net.au> rewrote the INLINE_SYSCALL macro */

#ifndef _LINUX_POWERPC_SYSDEP_H
#define _LINUX_POWERPC_SYSDEP_H 1

#include <sysdeps/unix/powerpc/sysdep.h>

/* Define __set_errno() for INLINE_SYSCALL macro below.  */
#ifndef __ASSEMBLER__
#include <errno.h>
#endif

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

/* This seems to always be the case on PPC.  */
#define ALIGNARG(log2) log2
/* For ELF we need the `.type' directive to make shared libs work right.  */
#define ASM_TYPE_DIRECTIVE(name,typearg) .type name,typearg;
#define ASM_SIZE_DIRECTIVE(name) .size name,.-name

#endif	/* __ASSEMBLER__ */

#undef INLINE_SYSCALL
#if 1
#define INLINE_SYSCALL(name, nr, args...)	\
  ({						\
    DECLARGS_##nr;				\
    long ret, err;				\
    LOADARGS_##nr(name, args);			\
    __asm __volatile ("sc\n\t"			\
		      "mfcr	%1\n\t"		\
		      : "=r" (r3), "=r" (err)	\
		      : ASM_INPUT_##nr		\
		      : "cc", "memory");	\
    ret = r3;					\
    if (err & 1 << 28)				\
      {						\
	__set_errno (ret);			\
	ret = -1L;				\
      }						\
    ret;					\
  })

#define DECLARGS_0 register long r0 __asm__ ("r0");	\
		   register long r3 __asm__ ("r3")
#define DECLARGS_1 DECLARGS_0
#define DECLARGS_2 DECLARGS_1; register long r4 __asm__ ("r4")
#define DECLARGS_3 DECLARGS_2; register long r5 __asm__ ("r5")
#define DECLARGS_4 DECLARGS_3; register long r6 __asm__ ("r6")
#define DECLARGS_5 DECLARGS_4; register long r7 __asm__ ("r7")
#define DECLARGS_6 DECLARGS_5; register long r8 __asm__ ("r8")

#define LOADARGS_0(name) \
	r0 = __NR_##name
#define LOADARGS_1(name, arg1) \
	LOADARGS_0(name); \
	r3 = (long) (arg1)
#define LOADARGS_2(name, arg1, arg2) \
	LOADARGS_1(name, arg1); \
	r4 = (long) (arg2)
#define LOADARGS_3(name, arg1, arg2, arg3) \
	LOADARGS_2(name, arg1, arg2); \
	r5 = (long) (arg3)
#define LOADARGS_4(name, arg1, arg2, arg3, arg4) \
	LOADARGS_3(name, arg1, arg2, arg3); \
	r6 = (long) (arg4)
#define LOADARGS_5(name, arg1, arg2, arg3, arg4, arg5) \
	LOADARGS_4(name, arg1, arg2, arg3, arg4); \
	r7 = (long) (arg5)
#define LOADARGS_6(name, arg1, arg2, arg3, arg4, arg5, arg6) \
	LOADARGS_5(name, arg1, arg2, arg3, arg4, arg5); \
	r8 = (long) (arg6)

#define ASM_INPUT_0 "r" (r0)
#define ASM_INPUT_1 ASM_INPUT_0, "0" (r3)
#define ASM_INPUT_2 ASM_INPUT_1, "r" (r4)
#define ASM_INPUT_3 ASM_INPUT_2, "r" (r5)
#define ASM_INPUT_4 ASM_INPUT_3, "r" (r6)
#define ASM_INPUT_5 ASM_INPUT_4, "r" (r7)
#define ASM_INPUT_6 ASM_INPUT_5, "r" (r8)

#else
/* This version is for kernels that implement system calls that
   behave like function calls as far as register saving.  */
#define INLINE_SYSCALL(name, nr, args...)			\
  ({								\
    register long r0 __asm__ ("r0");				\
    register long r3 __asm__ ("r3");				\
    register long r4 __asm__ ("r4");				\
    register long r5 __asm__ ("r5");				\
    register long r6 __asm__ ("r6");				\
    register long r7 __asm__ ("r7");				\
    register long r8 __asm__ ("r8");				\
    long ret, err;						\
    LOADARGS_##nr(name, args);					\
    __asm __volatile ("sc\n\t"					\
		      "mfcr	%7\n\t"				\
		      : "=r" (r0), "=r" (r3), "=r" (r4),	\
		        "=r" (r5), "=r" (r6), "=r" (r7),	\
		        "=r" (r8), "=r" (err)			\
		      : ASM_INPUT_##nr				\
		      : "r9", "r10", "r11", "r12",		\
		        "fr0", "fr1", "fr2", "fr3",		\
			"fr4", "fr5", "fr6", "fr7",		\
			"fr8", "fr9", "fr10", "fr11",		\
			"fr12", "fr13",				\
			"ctr", "lr",				\
			"cr0", "cr1", "cr5", "cr6", "cr7",	\
			"memory");				\
    ret = r3;							\
    if (err & 1 << 28)						\
      {								\
	__set_errno (ret);					\
	ret = -1L;						\
      }								\
    ret;							\
  })

#define LOADARGS_0(name) \
	r0 = __NR_##name
#define LOADARGS_1(name, arg1) \
	LOADARGS_0(name); \
	r3 = (long) (arg1)
#define LOADARGS_2(name, arg1, arg2) \
	LOADARGS_1(name, arg1); \
	r4 = (long) (arg2)
#define LOADARGS_3(name, arg1, arg2, arg3) \
	LOADARGS_2(name, arg1, arg2); \
	r5 = (long) (arg3)
#define LOADARGS_4(name, arg1, arg2, arg3, arg4) \
	LOADARGS_3(name, arg1, arg2, arg3); \
	r6 = (long) (arg4)
#define LOADARGS_5(name, arg1, arg2, arg3, arg4, arg5) \
	LOADARGS_4(name, arg1, arg2, arg3, arg4); \
	r7 = (long) (arg5)
#define LOADARGS_6(name, arg1, arg2, arg3, arg4, arg5, arg6) \
	LOADARGS_5(name, arg1, arg2, arg3, arg4, arg5); \
	r8 = (long) (arg6)

#define ASM_INPUT_0 "0" (r0)
#define ASM_INPUT_1 ASM_INPUT_0, "1" (r3)
#define ASM_INPUT_2 ASM_INPUT_1, "2" (r4)
#define ASM_INPUT_3 ASM_INPUT_2, "3" (r5)
#define ASM_INPUT_4 ASM_INPUT_3, "4" (r6)
#define ASM_INPUT_5 ASM_INPUT_4, "5" (r7)
#define ASM_INPUT_6 ASM_INPUT_5, "6" (r8)

#endif

#endif /* linux/powerpc/powerpc64/sysdep.h */
