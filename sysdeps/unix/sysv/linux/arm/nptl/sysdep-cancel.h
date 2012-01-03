/* Copyright (C) 2003, 2004, 2005, 2007, 2008, 2012
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

#include <sysdep.h>
#include <tls.h>
#ifndef __ASSEMBLER__
# include <nptl/pthreadP.h>
#endif

#if !defined NOT_IN_libc || defined IS_IN_libpthread || defined IS_IN_librt

# undef PSEUDO
# define PSEUDO(name, syscall_name, args)				\
  .section ".text";							\
    PSEUDO_PROLOGUE;							\
  ENTRY (__##syscall_name##_nocancel);					\
    DO_CALL (syscall_name, args);					\
    PSEUDO_RET;								\
  END (__##syscall_name##_nocancel);					\
  ENTRY (name);								\
    SINGLE_THREAD_P;							\
    DOARGS_##args;							\
    bne .Lpseudo_cancel;						\
    DO_CALL (syscall_name, 0);						\
    UNDOARGS_##args;							\
    cmn r0, $4096;							\
    PSEUDO_RET;								\
  .Lpseudo_cancel:							\
    DOCARGS_##args;	/* save syscall args etc. around CENABLE.  */	\
    CENABLE;								\
    mov ip, r0;		/* put mask in safe place.  */			\
    UNDOCARGS_##args;	/* restore syscall args.  */			\
    swi SYS_ify (syscall_name);	/* do the call.  */			\
    str r0, [sp, $-4]!; /* save syscall return value.  */		\
    mov r0, ip;		/* get mask back.  */				\
    CDISABLE;								\
    ldmfd sp!, {r0, lr}; /* retrieve return value and address.  */	\
    UNDOARGS_##args;							\
    cmn r0, $4096;

# define DOCARGS_0	str lr, [sp, #-4]!;
# define UNDOCARGS_0

# define DOCARGS_1	stmfd sp!, {r0, lr};
# define UNDOCARGS_1	ldr r0, [sp], #4;

# define DOCARGS_2	stmfd sp!, {r0, r1, lr};
# define UNDOCARGS_2	ldmfd sp!, {r0, r1};

# define DOCARGS_3	stmfd sp!, {r0, r1, r2, lr};
# define UNDOCARGS_3	ldmfd sp!, {r0, r1, r2};

# define DOCARGS_4	stmfd sp!, {r0, r1, r2, r3, lr};
# define UNDOCARGS_4	ldmfd sp!, {r0, r1, r2, r3};

# define DOCARGS_5	DOCARGS_4
# define UNDOCARGS_5	UNDOCARGS_4

# define DOCARGS_6	DOCARGS_5
# define UNDOCARGS_6	UNDOCARGS_5

# ifdef IS_IN_libpthread
#  define CENABLE	bl PLTJMP(__pthread_enable_asynccancel)
#  define CDISABLE	bl PLTJMP(__pthread_disable_asynccancel)
#  define __local_multiple_threads __pthread_multiple_threads
# elif !defined NOT_IN_libc
#  define CENABLE	bl PLTJMP(__libc_enable_asynccancel)
#  define CDISABLE	bl PLTJMP(__libc_disable_asynccancel)
#  define __local_multiple_threads __libc_multiple_threads
# elif defined IS_IN_librt
#  define CENABLE	bl PLTJMP(__librt_enable_asynccancel)
#  define CDISABLE	bl PLTJMP(__librt_disable_asynccancel)
# else
#  error Unsupported library
# endif

# if defined IS_IN_libpthread || !defined NOT_IN_libc
#  ifndef __ASSEMBLER__
extern int __local_multiple_threads attribute_hidden;
#   define SINGLE_THREAD_P __builtin_expect (__local_multiple_threads == 0, 1)
#  else
#   define SINGLE_THREAD_P						\
  ldr ip, 1b;								\
2:									\
  ldr ip, [pc, ip];							\
  teq ip, #0;
#   define PSEUDO_PROLOGUE						\
  1:  .word __local_multiple_threads - 2f - 8;
#  endif
# else
/*  There is no __local_multiple_threads for librt, so use the TCB.  */
#  ifndef __ASSEMBLER__
#   define SINGLE_THREAD_P						\
  __builtin_expect (THREAD_GETMEM (THREAD_SELF,				\
				   header.multiple_threads) == 0, 1)
#  else
#   define PSEUDO_PROLOGUE
#   define SINGLE_THREAD_P						\
  stmfd	sp!, {r0, lr};							\
  bl	__aeabi_read_tp;						\
  ldr	ip, [r0, #MULTIPLE_THREADS_OFFSET];				\
  ldmfd	sp!, {r0, lr};							\
  teq	ip, #0
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
