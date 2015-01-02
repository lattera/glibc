/* Copyright (C) 2003-2015 Free Software Foundation, Inc.

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

#if IS_IN (libc) || IS_IN (libpthread) || IS_IN (librt)

# undef PSEUDO
# define PSEUDO(name, syscall_name, args)				\
	.section ".text";						\
ENTRY (__##syscall_name##_nocancel);					\
.Lpseudo_nocancel:							\
	DO_CALL (syscall_name, args);					\
.Lpseudo_finish:							\
	cmn	x0, 4095;						\
	b.cs	.Lsyscall_error;					\
	.subsection 2;							\
	.size __##syscall_name##_nocancel,.-__##syscall_name##_nocancel; \
ENTRY (name);								\
	SINGLE_THREAD_P(16);						\
	cbz	w16, .Lpseudo_nocancel;					\
	/* Setup common stack frame no matter the number of args.	\
	   Also save the first arg, since it's basically free.  */	\
	stp	x30, x0, [sp, -64]!;					\
	cfi_adjust_cfa_offset (64);					\
	cfi_rel_offset (x30, 0);					\
	DOCARGS_##args;		/* save syscall args around CENABLE.  */ \
	CENABLE;							\
	mov	x16, x0;	/* save mask around syscall.  */	\
	UNDOCARGS_##args;	/* restore syscall args.  */		\
	DO_CALL (syscall_name, args);					\
	str	x0, [sp, 8];	/* save result around CDISABLE.  */	\
	mov	x0, x16;	/* restore mask for CDISABLE.  */	\
	CDISABLE;							\
	/* Break down the stack frame, restoring result at once.  */	\
	ldp	x30, x0, [sp], 64;					\
	cfi_adjust_cfa_offset (-64);					\
	cfi_restore (x30);						\
	b	.Lpseudo_finish;					\
	cfi_endproc;							\
	.size	name, .-name;						\
	.previous

# undef PSEUDO_END
# define PSEUDO_END(name)						\
	SYSCALL_ERROR_HANDLER;						\
	cfi_endproc

# define DOCARGS_0
# define DOCARGS_1
# define DOCARGS_2	str x1, [sp, 16]
# define DOCARGS_3	stp x1, x2, [sp, 16]
# define DOCARGS_4	DOCARGS_3; str x3, [sp, 32]
# define DOCARGS_5	DOCARGS_3; stp x3, x4, [sp, 32]
# define DOCARGS_6	DOCARGS_5; str x5, [sp, 48]

# define UNDOCARGS_0
# define UNDOCARGS_1	ldr x0, [sp, 8]
# define UNDOCARGS_2	ldp x0, x1, [sp, 8]
# define UNDOCARGS_3	UNDOCARGS_1; ldp x1, x2, [sp, 16]
# define UNDOCARGS_4	UNDOCARGS_2; ldp x2, x3, [sp, 24]
# define UNDOCARGS_5	UNDOCARGS_3; ldp x3, x4, [sp, 32]
# define UNDOCARGS_6	UNDOCARGS_4; ldp x4, x5, [sp, 40]

# if IS_IN (libpthread)
#  define CENABLE	bl __pthread_enable_asynccancel
#  define CDISABLE	bl __pthread_disable_asynccancel
#  define __local_multiple_threads __pthread_multiple_threads
# elif IS_IN (libc)
#  define CENABLE	bl __libc_enable_asynccancel
#  define CDISABLE	bl __libc_disable_asynccancel
#  define __local_multiple_threads __libc_multiple_threads
# elif IS_IN (librt)
#  define CENABLE	bl __librt_enable_asynccancel
#  define CDISABLE	bl __librt_disable_asynccancel
# else
#  error Unsupported library
# endif

# if IS_IN (libpthread) || IS_IN (libc)
#  ifndef __ASSEMBLER__
extern int __local_multiple_threads attribute_hidden;
#   define SINGLE_THREAD_P __builtin_expect (__local_multiple_threads == 0, 1)
#  else
#   define SINGLE_THREAD_P(R)						\
	adrp	x##R, __local_multiple_threads;				\
	ldr	w##R, [x##R, :lo12:__local_multiple_threads]
#  endif
# else
/*  There is no __local_multiple_threads for librt, so use the TCB.  */
#  ifndef __ASSEMBLER__
#   define SINGLE_THREAD_P						\
  __builtin_expect (THREAD_GETMEM (THREAD_SELF,				\
				   header.multiple_threads) == 0, 1)
#  else
#   define SINGLE_THREAD_P(R)						\
	mrs     x##R, tpidr_el0;					\
	sub	x##R, x##R, PTHREAD_SIZEOF;				\
	ldr	w##R, [x##R, PTHREAD_MULTIPLE_THREADS_OFFSET]
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
