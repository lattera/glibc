/* Copyright (C) 1992, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004
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

/* Some systen calls got renamed over time, but retained the same semantics.
   Handle them here so they can be catched by both C and assembler stubs in
   glibc.  */

#ifdef __NR_pread64
# ifdef __NR_pread
#  error "__NR_pread and __NR_pread64 both defined???"
# endif
# define __NR_pread __NR_pread64
#endif

#ifdef __NR_pwrite64
# ifdef __NR_pwrite
#  error "__NR_pwrite and __NR_pwrite64 both defined???"
# endif
# define __NR_pwrite __NR_pwrite64
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

/* This version is for kernels that implement system calls that
   behave like function calls as far as register saving.  */
#define INLINE_SYSCALL(name, nr, args...)				\
  ({									\
    INTERNAL_SYSCALL_DECL (sc_err);					\
    long int sc_ret = INTERNAL_SYSCALL (name, sc_err, nr, args);	\
    if (INTERNAL_SYSCALL_ERROR_P (sc_ret, sc_err))			\
      {									\
        __set_errno (INTERNAL_SYSCALL_ERRNO (sc_ret, sc_err));		\
        sc_ret = -1L;							\
      }									\
    sc_ret;								\
  })

/* Define a macro which expands inline into the wrapper code for a system
   call. This use is for internal calls that do not need to handle errors
   normally. It will never touch errno. This returns just what the kernel
   gave back in the non-error (CR0.SO cleared) case, otherwise (CR0.SO set)
   the negation of the return value in the kernel gets reverted.  */

#undef INTERNAL_SYSCALL
#define INTERNAL_SYSCALL_NCS(name, err, nr, args...) \
  ({									\
    register long int r0  __asm__ ("r0");				\
    register long int r3  __asm__ ("r3");				\
    register long int r4  __asm__ ("r4");				\
    register long int r5  __asm__ ("r5");				\
    register long int r6  __asm__ ("r6");				\
    register long int r7  __asm__ ("r7");				\
    register long int r8  __asm__ ("r8");				\
    LOADARGS_##nr(name, args);						\
    __asm__ __volatile__						\
      ("sc\n\t"								\
       "mfcr  %0\n\t"							\
       "0:"								\
       : "=&r" (r0),							\
         "=&r" (r3), "=&r" (r4), "=&r" (r5),				\
         "=&r" (r6), "=&r" (r7), "=&r" (r8)				\
       : ASM_INPUT_##nr							\
       : "r9", "r10", "r11", "r12",					\
         "cr0", "ctr", "memory");					\
	  err = r0;  \
    (int) r3;  \
  })
#define INTERNAL_SYSCALL(name, err, nr, args...)			\
  INTERNAL_SYSCALL_NCS (__NR_##name, err, nr, ##args)

#undef INTERNAL_SYSCALL_DECL
#define INTERNAL_SYSCALL_DECL(err) long int err

#undef INTERNAL_SYSCALL_ERROR_P
#define INTERNAL_SYSCALL_ERROR_P(val, err) \
  (__builtin_expect (err & (1 << 28), 0))

#undef INTERNAL_SYSCALL_ERRNO
#define INTERNAL_SYSCALL_ERRNO(val, err)     (val)

#define LOADARGS_0(name, dummy) \
	r0 = name
#define LOADARGS_1(name, __arg1) \
	long int arg1 = (long int) (__arg1); \
	LOADARGS_0(name, 0); \
	extern void __illegally_sized_syscall_arg1 (void); \
	if (__builtin_classify_type (__arg1) != 5 && sizeof (__arg1) > 8) \
	  __illegally_sized_syscall_arg1 (); \
	r3 = arg1
#define LOADARGS_2(name, __arg1, __arg2) \
	long int arg2 = (long int) (__arg2); \
	LOADARGS_1(name, __arg1); \
	extern void __illegally_sized_syscall_arg2 (void); \
	if (__builtin_classify_type (__arg2) != 5 && sizeof (__arg2) > 8) \
	  __illegally_sized_syscall_arg2 (); \
	r4 = arg2
#define LOADARGS_3(name, __arg1, __arg2, __arg3) \
	long int arg3 = (long int) (__arg3); \
	LOADARGS_2(name, __arg1, __arg2); \
	extern void __illegally_sized_syscall_arg3 (void); \
	if (__builtin_classify_type (__arg3) != 5 && sizeof (__arg3) > 8) \
	  __illegally_sized_syscall_arg3 (); \
	r5 = arg3
#define LOADARGS_4(name, __arg1, __arg2, __arg3, __arg4) \
	long int arg4 = (long int) (__arg4); \
	LOADARGS_3(name, __arg1, __arg2, __arg3); \
	extern void __illegally_sized_syscall_arg4 (void); \
	if (__builtin_classify_type (__arg4) != 5 && sizeof (__arg4) > 8) \
	  __illegally_sized_syscall_arg4 (); \
	r6 = arg4
#define LOADARGS_5(name, __arg1, __arg2, __arg3, __arg4, __arg5) \
	long int arg5 = (long int) (__arg5); \
	LOADARGS_4(name, __arg1, __arg2, __arg3, __arg4); \
	extern void __illegally_sized_syscall_arg5 (void); \
	if (__builtin_classify_type (__arg5) != 5 && sizeof (__arg5) > 8) \
	  __illegally_sized_syscall_arg5 (); \
	r7 = arg5
#define LOADARGS_6(name, __arg1, __arg2, __arg3, __arg4, __arg5, __arg6) \
	long int arg6 = (long int) (__arg6); \
	LOADARGS_5(name, __arg1, __arg2, __arg3, __arg4, __arg5); \
	extern void __illegally_sized_syscall_arg6 (void); \
	if (__builtin_classify_type (__arg6) != 5 && sizeof (__arg6) > 8) \
	  __illegally_sized_syscall_arg6 (); \
	r8 = arg6

#define ASM_INPUT_0 "0" (r0)
#define ASM_INPUT_1 ASM_INPUT_0, "1" (r3)
#define ASM_INPUT_2 ASM_INPUT_1, "2" (r4)
#define ASM_INPUT_3 ASM_INPUT_2, "3" (r5)
#define ASM_INPUT_4 ASM_INPUT_3, "4" (r6)
#define ASM_INPUT_5 ASM_INPUT_4, "5" (r7)
#define ASM_INPUT_6 ASM_INPUT_5, "6" (r8)

#endif /* linux/powerpc/powerpc64/sysdep.h */
