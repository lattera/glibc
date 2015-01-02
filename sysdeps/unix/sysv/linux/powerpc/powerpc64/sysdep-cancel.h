/* Cancellable system call stubs.  Linux/PowerPC64 version.
   Copyright (C) 2003-2015 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Franz Sirl <Franz.Sirl-kernel@lauterbach.com>, 2003.

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

# ifdef HAVE_ASM_GLOBAL_DOT_NAME
#  define DASHDASHPFX(str) .__##str
# else
#  define DASHDASHPFX(str) __##str
# endif

#if _CALL_ELF == 2
#define CANCEL_FRAMESIZE (FRAME_MIN_SIZE+16+48)
#define CANCEL_PARM_SAVE (FRAME_MIN_SIZE+16)
#else
#define CANCEL_FRAMESIZE (FRAME_MIN_SIZE+16)
#define CANCEL_PARM_SAVE (CANCEL_FRAMESIZE+FRAME_PARM_SAVE)
#endif

# undef PSEUDO
# define PSEUDO(name, syscall_name, args)				\
  .section ".text";							\
  ENTRY (name)								\
    SINGLE_THREAD_P;							\
    bne- .Lpseudo_cancel;						\
  .type DASHDASHPFX(syscall_name##_nocancel),@function;			\
  .globl DASHDASHPFX(syscall_name##_nocancel);				\
  DASHDASHPFX(syscall_name##_nocancel):					\
    DO_CALL (SYS_ify (syscall_name));					\
    PSEUDO_RET;								\
  .size DASHDASHPFX(syscall_name##_nocancel),.-DASHDASHPFX(syscall_name##_nocancel);	\
  .Lpseudo_cancel:							\
    stdu 1,-CANCEL_FRAMESIZE(1);					\
    cfi_adjust_cfa_offset (CANCEL_FRAMESIZE);				\
    mflr 9;								\
    std  9,CANCEL_FRAMESIZE+FRAME_LR_SAVE(1);				\
    cfi_offset (lr, FRAME_LR_SAVE);					\
    DOCARGS_##args;	/* save syscall args around CENABLE.  */	\
    CENABLE;								\
    std  3,FRAME_MIN_SIZE(1); /* store CENABLE return value (MASK).  */	\
    UNDOCARGS_##args;	/* restore syscall args.  */			\
    DO_CALL (SYS_ify (syscall_name));					\
    mfcr 0;		/* save CR/R3 around CDISABLE.  */		\
    std  3,FRAME_MIN_SIZE+8(1);						\
    std  0,CANCEL_FRAMESIZE+FRAME_CR_SAVE(1);				\
    cfi_offset (cr, FRAME_CR_SAVE);					\
    ld   3,FRAME_MIN_SIZE(1); /* pass MASK to CDISABLE.  */		\
    CDISABLE;								\
    ld   9,CANCEL_FRAMESIZE+FRAME_LR_SAVE(1);				\
    ld   0,CANCEL_FRAMESIZE+FRAME_CR_SAVE(1); /* restore CR/R3. */	\
    ld   3,FRAME_MIN_SIZE+8(1);						\
    mtlr 9;								\
    mtcr 0;								\
    addi 1,1,CANCEL_FRAMESIZE;						\
    cfi_adjust_cfa_offset (-CANCEL_FRAMESIZE);				\
    cfi_restore (lr);							\
    cfi_restore (cr)

# define DOCARGS_0
# define UNDOCARGS_0

# define DOCARGS_1	std 3,CANCEL_PARM_SAVE(1); DOCARGS_0
# define UNDOCARGS_1	ld 3,CANCEL_PARM_SAVE(1); UNDOCARGS_0

# define DOCARGS_2	std 4,CANCEL_PARM_SAVE+8(1); DOCARGS_1
# define UNDOCARGS_2	ld 4,CANCEL_PARM_SAVE+8(1); UNDOCARGS_1

# define DOCARGS_3	std 5,CANCEL_PARM_SAVE+16(1); DOCARGS_2
# define UNDOCARGS_3	ld 5,CANCEL_PARM_SAVE+16(1); UNDOCARGS_2

# define DOCARGS_4	std 6,CANCEL_PARM_SAVE+24(1); DOCARGS_3
# define UNDOCARGS_4	ld 6,CANCEL_PARM_SAVE+24(1); UNDOCARGS_3

# define DOCARGS_5	std 7,CANCEL_PARM_SAVE+32(1); DOCARGS_4
# define UNDOCARGS_5	ld 7,CANCEL_PARM_SAVE+32(1); UNDOCARGS_4

# define DOCARGS_6	std 8,CANCEL_PARM_SAVE+40(1); DOCARGS_5
# define UNDOCARGS_6	ld 8,CANCEL_PARM_SAVE+40(1); UNDOCARGS_5

# if IS_IN (libpthread)
#  ifdef SHARED
#   define CENABLE	bl JUMPTARGET(__pthread_enable_asynccancel)
#   define CDISABLE	bl JUMPTARGET(__pthread_disable_asynccancel)
#  else
#   define CENABLE	bl JUMPTARGET(__pthread_enable_asynccancel); nop
#   define CDISABLE	bl JUMPTARGET(__pthread_disable_asynccancel); nop
#  endif
# elif IS_IN (libc)
#  ifdef SHARED
#   define CENABLE	bl JUMPTARGET(__libc_enable_asynccancel)
#   define CDISABLE	bl JUMPTARGET(__libc_disable_asynccancel)
#  else
#   define CENABLE	bl JUMPTARGET(__libc_enable_asynccancel); nop
#   define CDISABLE	bl JUMPTARGET(__libc_disable_asynccancel); nop
#  endif
# elif IS_IN (librt)
#  ifdef SHARED
#   define CENABLE	bl JUMPTARGET(__librt_enable_asynccancel)
#   define CDISABLE	bl JUMPTARGET(__librt_disable_asynccancel)
#  else
#   define CENABLE	bl JUMPTARGET(__librt_enable_asynccancel); nop
#   define CDISABLE	bl JUMPTARGET(__librt_disable_asynccancel); nop
#  endif
# else
#  error Unsupported library
# endif

# ifndef __ASSEMBLER__
#  define SINGLE_THREAD_P						\
  __builtin_expect (THREAD_GETMEM (THREAD_SELF,				\
				   header.multiple_threads) == 0, 1)
# else
#   define SINGLE_THREAD_P						\
  lwz   10,MULTIPLE_THREADS_OFFSET(13);				\
  cmpwi 10,0
# endif

#elif !defined __ASSEMBLER__

# define SINGLE_THREAD_P (1)
# define NO_CANCELLATION 1

#endif

#ifndef __ASSEMBLER__
# define RTLD_SINGLE_THREAD_P \
  __builtin_expect (THREAD_GETMEM (THREAD_SELF, \
				   header.multiple_threads) == 0, 1)
#endif
