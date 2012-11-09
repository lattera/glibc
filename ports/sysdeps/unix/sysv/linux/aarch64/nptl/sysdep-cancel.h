/* Copyright (C) 2003-2012 Free Software Foundation, Inc.

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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <sysdep.h>
#include <tls.h>
#ifndef __ASSEMBLER__
# include <nptl/pthreadP.h>
#endif

#if !defined NOT_IN_libc || defined IS_IN_libpthread || defined IS_IN_librt

# undef PSEUDO
# define PSEUDO(name, syscall_name, args)				\
  .section ".text";							\
  .type __##syscall_name##_nocancel,%function;				\
  .globl __##syscall_name##_nocancel;					\
  __##syscall_name##_nocancel:						\
    cfi_startproc;							\
    DO_CALL (syscall_name, args);					\
    PSEUDO_RET;								\
    cfi_endproc;							\
    .size __##syscall_name##_nocancel,.-__##syscall_name##_nocancel;	\
  ENTRY (name);								\
    SINGLE_THREAD_P;							\
    DOARGS_##args;							\
    bne .Lpseudo_cancel;						\
    DO_CALL (syscall_name, 0);						\
    UNDOARGS_##args;							\
    cmn x0, 4095;							\
    PSEUDO_RET;								\
  .Lpseudo_cancel:							\
    DOCARGS_##args;	/* save syscall args etc. around CENABLE.  */	\
    CENABLE;								\
    mov x16, x0;	/* put mask in safe place.  */			\
    UNDOCARGS_##args;	/* restore syscall args.  */			\
    mov x8, SYS_ify (syscall_name);	/* do the call.  */		\
    svc	0;								\
    str x0, [sp, -16]!;	/* save syscall return value.  */		\
    cfi_adjust_cfa_offset (16);						\
    mov x0, x16;	 /* get mask back.  */				\
    CDISABLE;								\
    ldr x0, [sp], 16;							\
    cfi_adjust_cfa_offset (-16);					\
    ldr x30, [sp], 16;							\
    cfi_adjust_cfa_offset (-16);					\
    cfi_restore (x30);							\
    UNDOARGS_##args;							\
    cmn x0, 4095;

# define DOCARGS_0							\
	str x30, [sp, -16]!;						\
	cfi_adjust_cfa_offset (16);					\
	cfi_rel_offset (x30, 0)

# define UNDOCARGS_0

# define DOCARGS_1							\
	DOCARGS_0;							\
	str x0, [sp, -16]!;						\
	cfi_adjust_cfa_offset (16);					\
	cfi_rel_offset (x0, 0)

# define UNDOCARGS_1							\
	ldr x0, [sp], 16;						\
	cfi_restore (x0);						\
	cfi_adjust_cfa_offset (-16);					\

# define DOCARGS_2							\
	DOCARGS_1;							\
	str x1, [sp, -16]!;						\
	cfi_adjust_cfa_offset (16);					\
	cfi_rel_offset (x1, 0)

# define UNDOCARGS_2							\
	ldr x1, [sp], 16;						\
	cfi_restore (x1);						\
	cfi_adjust_cfa_offset (-16);					\
	UNDOCARGS_1

# define DOCARGS_3							\
	DOCARGS_2;							\
	str x2, [sp, -16]!;						\
	cfi_adjust_cfa_offset (16);					\
	cfi_rel_offset (x2, 0)

# define UNDOCARGS_3							\
	ldr x2, [sp], 16;						\
	cfi_restore (x2);						\
	cfi_adjust_cfa_offset (-16);					\
	UNDOCARGS_2

# define DOCARGS_4							\
	DOCARGS_3;							\
	str x3, [sp, -16]!;						\
	cfi_adjust_cfa_offset (16);					\
	cfi_rel_offset (x3, 0)

# define UNDOCARGS_4							\
	ldr x3, [sp], 16;						\
	cfi_restore (x3);						\
	cfi_adjust_cfa_offset (-16);					\
	UNDOCARGS_3

# define DOCARGS_5							\
	DOCARGS_4;							\
	str x4, [sp, -16]!;						\
	cfi_adjust_cfa_offset (16);					\
	cfi_rel_offset (x4, 0)

# define UNDOCARGS_5							\
	ldr x4, [sp], 16;						\
	cfi_restore (x4);						\
	cfi_adjust_cfa_offset (-16);					\
	UNDOCARGS_4

# define DOCARGS_6							\
	DOCARGS_5;							\
	str x5, [sp, -16]!;						\
	cfi_adjust_cfa_offset (16);					\
	cfi_rel_offset (x5, 0)

# define UNDOCARGS_6							\
	ldr x5, [sp], 16;						\
	cfi_restore (x5);						\
	cfi_adjust_cfa_offset (-16);					\
	UNDOCARGS_5

# ifdef IS_IN_libpthread
#  define CENABLE	bl __pthread_enable_asynccancel
#  define CDISABLE	bl __pthread_disable_asynccancel
#  define __local_multiple_threads __pthread_multiple_threads
# elif !defined NOT_IN_libc
#  define CENABLE	bl __libc_enable_asynccancel
#  define CDISABLE	bl __libc_disable_asynccancel
#  define __local_multiple_threads __libc_multiple_threads
# elif defined IS_IN_librt
#  define CENABLE	bl __librt_enable_asynccancel
#  define CDISABLE	bl __librt_disable_asynccancel
# else
#  error Unsupported library
# endif

# if defined IS_IN_libpthread || !defined NOT_IN_libc
#  ifndef __ASSEMBLER__
extern int __local_multiple_threads attribute_hidden;
#   define SINGLE_THREAD_P __builtin_expect (__local_multiple_threads == 0, 1)
#  else
#   define SINGLE_THREAD_P						\
  adrp	x16, __local_multiple_threads;					\
  add	x16, x16, #:lo12:__local_multiple_threads;			\
  ldr	x16, [x16];							\
  cmp	x16, 0;
#  endif
# else
/*  There is no __local_multiple_threads for librt, so use the TCB.  */
#  ifndef __ASSEMBLER__
#   define SINGLE_THREAD_P						\
  __builtin_expect (THREAD_GETMEM (THREAD_SELF,				\
				   header.multiple_threads) == 0, 1)
#  else
#   define SINGLE_THREAD_P						\
  stp	x0, x30, [sp, -16]!;						\
  cfi_adjust_cfa_offset (16);						\
  cfi_rel_offset (x0, 0);						\
  cfi_rel_offset (x30, 8);						\
  bl	__read_tp;							\
  sub	x0, x0, PTHREAD_SIZEOF;						\
  ldr	x16, [x0, PTHREAD_MULTIPLE_THREADS_OFFSET];			\
  ldp	x0, x30, [sp], 16;						\
  cfi_restore (x0);							\
  cfi_restore (x30);							\
  cfi_adjust_cfa_offset (-16);						\
  cmp	x16, 0
#   define SINGLE_THREAD_P_PIC(x) SINGLE_THREAD_P
#  endif
# endif

#elif !defined __ASSEMBLER__

/* For rtld, et cetera.  */
# define SINGLE_THREAD_P 1
# define NO_CANCELLATION 1

#endif

#ifndef __ASSEMBLER__
# define RTLD_SINGLE_THREAD_P \
  __builtin_expect (THREAD_GETMEM (THREAD_SELF, \
				   header.multiple_threads) == 0, 1)
#endif
