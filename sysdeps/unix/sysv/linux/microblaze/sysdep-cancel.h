/* Copyright (C) 2014-2016 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include <sysdep.h>
#include <tls.h>
#ifndef __ASSEMBLER__
# include <nptl/pthreadP.h>
#endif

#if IS_IN (libc) || IS_IN (libpthread) || IS_IN (librt)

# if !IS_IN (librt) || !defined(PIC)
#  define AC_STACK_SIZE  16  /* space for r15, async_cancel arg and 2 temp words */
#  define AC_SET_GOT /* empty */
#  define AC_RESTORE_GOT /* empty */
# else
#  define AC_STACK_SIZE  20  /* extra 4 bytes for r20 */
#  define AC_SET_GOT                                                 \
    swi   r20, r1, AC_STACK_SIZE-4;                                  \
    mfs   r20, rpc;                                                  \
    addik r20, r20, _GLOBAL_OFFSET_TABLE_+8;
#  define AC_RESTORE_GOT                                             \
    lwi   r20, r1, AC_STACK_SIZE-4;
# endif

# undef PSEUDO
# define PSEUDO(name, syscall_name, args)                            \
  .text;                                                             \
  ENTRY (name)                                                       \
    SINGLE_THREAD_P(r12);                                            \
    bnei r12, L(pseudo_cancel);                                      \
  .globl __##syscall_name##_nocancel;                                \
  .type __##syscall_name##_nocancel,@function;                       \
__##syscall_name##_nocancel:                                         \
    DO_CALL (syscall_name, args);                                    \
    addik r4, r0, -4095;                                             \
    cmpu  r4, r4, r3;                                                \
    bgei  r4, SYSCALL_ERROR_LABEL;                                   \
    rtsd  r15, 8;                                                    \
    nop;                                                             \
  .size __##syscall_name##_nocancel, .-__##syscall_name##_nocancel;  \
L(pseudo_cancel):                                                    \
    addik r1, r1, -AC_STACK_SIZE;                                    \
    swi   r15, r1, 0;                                                \
    AC_SET_GOT                                                       \
    DOCARGS_##args                                                   \
    CENABLE;                                                         \
    swi   r3, r1, 8;                                                 \
    UNDOCARGS_##args                                                 \
    DO_CALL (syscall_name, args);                                    \
    swi   r3, r1, 12;                                                \
    lwi   r5, r1, 8;                                                 \
    CDISABLE;                                                        \
    lwi   r3, r1, 12;                                                \
    lwi   r15, r1, 0;                                                \
    AC_RESTORE_GOT                                                   \
    addik r1, r1, AC_STACK_SIZE;                                     \
    addik r4, r0, -4095;                                             \
    cmpu  r4, r4, r3;                                                \
    bgei  r4, SYSCALL_ERROR_LABEL;                                   \
    rtsd  r15, 8;                                                    \
    nop;

/*
 * Macros to save/restore syscall arguments across CENABLE
 * The arguments are saved into the caller's stack (original r1 + 4)
 */

# define DOCARGS_0
# define DOCARGS_1  swi   r5, r1, AC_STACK_SIZE + 4;
# define DOCARGS_2  swi   r6, r1, AC_STACK_SIZE + 8; DOCARGS_1
# define DOCARGS_3  swi   r7, r1, AC_STACK_SIZE + 12; DOCARGS_2
# define DOCARGS_4  swi   r8, r1, AC_STACK_SIZE + 16; DOCARGS_3
# define DOCARGS_5  swi   r9, r1, AC_STACK_SIZE + 20; DOCARGS_4
# define DOCARGS_6  swi   r10, r1, AC_STACK_SIZE + 24; DOCARGS_5

# define UNDOCARGS_0
# define UNDOCARGS_1  lwi   r5, r1, AC_STACK_SIZE + 4;
# define UNDOCARGS_2  UNDOCARGS_1 lwi   r6, r1, AC_STACK_SIZE + 8;
# define UNDOCARGS_3  UNDOCARGS_2 lwi   r7, r1, AC_STACK_SIZE + 12;
# define UNDOCARGS_4  UNDOCARGS_3 lwi   r8, r1, AC_STACK_SIZE + 16;
# define UNDOCARGS_5  UNDOCARGS_4 lwi   r9, r1, AC_STACK_SIZE + 20;
# define UNDOCARGS_6  UNDOCARGS_5 lwi   r10, r1, AC_STACK_SIZE + 24;

# ifdef PIC
#  define PSEUDO_JMP(sym)  brlid r15, sym##@PLTPC; addk r0, r0, r0
# else
#  define PSEUDO_JMP(sym)  brlid r15, sym; addk r0, r0, r0
# endif

# if IS_IN (libpthread)
#  define CENABLE PSEUDO_JMP (__pthread_enable_asynccancel)
#  define CDISABLE  PSEUDO_JMP (__pthread_disable_asynccancel)
#  define __local_multiple_threads __pthread_multiple_threads
# elif IS_IN (libc)
#  define CENABLE PSEUDO_JMP (__libc_enable_asynccancel)
#  define CDISABLE  PSEUDO_JMP (__libc_disable_asynccancel)
#  define __local_multiple_threads __libc_multiple_threads
# elif IS_IN (librt)
#  define CENABLE PSEUDO_JMP (__librt_enable_asynccancel)
#  define CDISABLE  PSEUDO_JMP (__librt_disable_asynccancel)
# else
#  error Unsupported library
# endif


# if IS_IN (libpthread) || IS_IN (libc)
#  ifndef __ASSEMBLER__
extern int __local_multiple_threads attribute_hidden;
#   define SINGLE_THREAD_P __builtin_expect (__local_multiple_threads == 0, 1)
#  else
#   if !defined PIC
#    define SINGLE_THREAD_P(reg) lwi reg, r0, __local_multiple_threads;
#   else
#    define SINGLE_THREAD_P(reg)                                     \
      mfs   reg, rpc;                                                \
      addik reg, reg, _GLOBAL_OFFSET_TABLE_+8;                       \
      lwi   reg, reg, __local_multiple_threads@GOT;                  \
      lwi   reg, reg, 0;
#   endif
#  endif
# else
#  ifndef __ASSEMBLER__
#   define SINGLE_THREAD_P                                           \
  __builtin_expect (THREAD_GETMEM (THREAD_SELF,                      \
                                   header.multiple_threads) == 0, 1)
#  else
#   define SINGLE_THREAD_P(reg)                                      \
     lwi reg, r0, MULTIPLE_THREADS_OFFSET(reg)
#  endif
# endif

#elif !defined __ASSEMBLER__

# define SINGLE_THREAD_P (1)
# define NO_CANCELLATION (1)

#endif

#ifndef __ASSEMBLER__
# define RTLD_SINGLE_THREAD_P                                        \
  __builtin_expect (THREAD_GETMEM (THREAD_SELF,                      \
                                   header.multiple_threads) == 0, 1)
#endif
