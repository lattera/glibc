/* Copyright (C) 2010 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Maxim Kuvyrkov <maxim@codesourcery.com>, 2010.

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
# define PSEUDO(name, syscall_name, args)				      \
  .text;								      \
  ENTRY (name)								      \
    SINGLE_THREAD_P;							      \
    jne .Lpseudo_cancel;						      \
  .type __##syscall_name##_nocancel,@function;			              \
  .globl __##syscall_name##_nocancel;				 	      \
  __##syscall_name##_nocancel:					              \
    DO_CALL (syscall_name, args);					      \
    cmp.l &-4095, %d0;							      \
    jcc SYSCALL_ERROR_LABEL;						      \
    rts;								      \
  .size __##syscall_name##_nocancel,.-__##syscall_name##_nocancel;	      \
  .Lpseudo_cancel:							      \
    cfi_startproc;							      \
    CENABLE;								      \
    DOCARGS_##args							      \
    move.l %d0, -(%sp); /* Save result of CENABLE.  */  		      \
    cfi_adjust_cfa_offset (4); \
    move.l &SYS_ify (syscall_name), %d0;				      \
    trap &0;								      \
    move.l %d0, %d2;							      \
    CDISABLE;								      \
    addq.l &4, %sp; /* Remove result of CENABLE from the stack.  */           \
    cfi_adjust_cfa_offset (-4); \
    move.l %d2, %d0;							      \
    UNDOCARGS_##args							      \
    cmp.l &-4095, %d0;							      \
    jcc SYSCALL_ERROR_LABEL;		                                      \
    cfi_endproc

/* Note: we use D2 to save syscall's return value as D0 will be clobbered in
   CDISABLE.  */
# define DOCARGS_0	move.l %d2, -(%sp);		\
  cfi_adjust_cfa_offset (4); cfi_rel_offset (%d2, 0);
# define UNDOCARGS_0	move.l (%sp)+, %d2;	\
  cfi_adjust_cfa_offset (-4); cfi_restore (%d2);

# define DOCARGS_1	_DOCARGS_1 (4); DOCARGS_0
# define _DOCARGS_1(n)	move.l n(%sp), %d1;
# define UNDOCARGS_1	UNDOCARGS_0

# define DOCARGS_2	_DOCARGS_2 (8)
# define _DOCARGS_2(n)	DOCARGS_0 move.l n+4(%sp), %d2; _DOCARGS_1 (n)
# define UNDOCARGS_2	UNDOCARGS_0

/* TODO: We can optimize DOCARGS_{3, 4} by saving registers to a0 and a1
   instead of pushing them on stack.  */
# define DOCARGS_3	_DOCARGS_3 (12)
# define _DOCARGS_3(n)	move.l %d3, -(%sp);				\
  cfi_adjust_cfa_offset (4); cfi_rel_offset (%d3, 0);			\
  move.l n+4(%sp), %d3; _DOCARGS_2 (n)
# define UNDOCARGS_3	UNDOCARGS_2 move.l (%sp)+, %d3;		\
  cfi_adjust_cfa_offset (-4); cfi_restore (%d3);

# define DOCARGS_4	_DOCARGS_4 (16)
# define _DOCARGS_4(n)	move.l %d4, -(%sp);			\
  cfi_adjust_cfa_offset (4); cfi_rel_offset (%d4, 0);		\
  move.l n+4(%sp), %d4; _DOCARGS_3 (n)
# define UNDOCARGS_4	UNDOCARGS_3 move.l (%sp)+, %d4;	\
  cfi_adjust_cfa_offset (-4); cfi_restore (%d4);

# define DOCARGS_5	_DOCARGS_5 (20)
# define _DOCARGS_5(n)	move.l %d5, %a1; cfi_register (%d5, a1); \
  move.l n(%sp), %d5; _DOCARGS_4 (n-4)
# define UNDOCARGS_5	UNDOCARGS_4 move.l %a1, %d5; cfi_restore (%d5);

# define DOCARGS_6	_DOCARGS_6 (24)
# define _DOCARGS_6(n)	move.l n(%sp), %a0; _DOCARGS_5 (n-4)
# define UNDOCARGS_6	UNDOCARGS_5

# ifdef PIC
#  define PSEUDO_JMP(sym) jbsr sym ## @PLTPC
# else
#  define PSEUDO_JMP(sym) jbsr sym
# endif

# ifdef IS_IN_libpthread
#  define CENABLE	PSEUDO_JMP (__pthread_enable_asynccancel)
#  define CDISABLE	PSEUDO_JMP (__pthread_disable_asynccancel)
# elif !defined NOT_IN_libc
#  define CENABLE	PSEUDO_JMP (__libc_enable_asynccancel)
#  define CDISABLE	PSEUDO_JMP (__libc_disable_asynccancel)
# elif defined IS_IN_librt
#  define CENABLE	PSEUDO_JMP (__librt_enable_asynccancel)
#  define CDISABLE	PSEUDO_JMP (__librt_disable_asynccancel)
# else
#  error Unsupported library
# endif

# ifndef __ASSEMBLER__
#  define SINGLE_THREAD_P						\
  __builtin_expect (THREAD_GETMEM (THREAD_SELF,				\
				   header.multiple_threads) == 0, 1)
# else
#  define SINGLE_THREAD_P			\
  PSEUDO_JMP (__m68k_read_tp);		        \
  tst.l MULTIPLE_THREADS_OFFSET(%a0)
# endif

#elif !defined __ASSEMBLER__

# define SINGLE_THREAD_P (1)
# define NO_CANCELLATION (1)

#endif

#ifndef __ASSEMBLER__
# define RTLD_SINGLE_THREAD_P					  \
  __builtin_expect (THREAD_GETMEM (THREAD_SELF,			  \
				   header.multiple_threads) == 0, \
		    1)
#endif
