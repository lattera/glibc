/* Copyright (C) 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Jakub Jelinek <jakub@redhat.com>, 2003.

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
# include <linuxthreads/internals.h>
#endif

#if !defined NOT_IN_libc || defined IS_IN_libpthread

# undef PSEUDO
# define PSEUDO(name, syscall_name, args)				      \
	.text;								      \
ENTRY(name)								      \
	SINGLE_THREAD_P(%r1)						      \
	jne	L(pseudo_cancel);					      \
	DO_CALL(syscall_name, args);					      \
	lhi	%r4,-4095;						      \
	clr	%r2,%r4;						      \
	jnl	SYSCALL_ERROR_LABEL;					      \
	br	%r14;							      \
L(pseudo_cancel):							      \
	STM_##args							      \
	stm	%r13,%r15,52(%r15);					      \
	ahi	%r15,-96;						      \
	basr    %r13,0;							      \
200301:	l	%r1,200302f-200301b(%r13);				      \
	bas	%r14,0(%r13,%r1);					      \
	lr	%r0,%r2;						      \
	LM_##args							      \
	DO_CALL(syscall_name, args);					      \
	l	%r3,200303f-200301b(%r13);				      \
	lr	%r4,%r13;						      \
	lr	%r13,%r2;						      \
	lr	%r2,%r0;						      \
	bas	%r14,0(%r4,%r3);					      \
	lr	%r2,%r13;						      \
	lm	%r13,%r15,52+96(%r15);					      \
	lhi	%r4,-4095;						      \
	clr	%r2,%r4;						      \
	jnl	SYSCALL_ERROR_LABEL;					      \
L(pseudo_end):

#undef PSEUDO_END
#define PSEUDO_END(name)						      \
  SYSCALL_ERROR_HANDLER;						      \
200302:	.long	CENABLE-200301b;					      \
200303:	.long	CDISABLE-200301b;					      \
    END (name)

# ifdef IS_IN_libpthread
#  define CENABLE	__pthread_enable_asynccancel
#  define CDISABLE	__pthread_disable_asynccancel
# else
#  define CENABLE	__libc_enable_asynccancel
#  define CDISABLE	__libc_disable_asynccancel
# endif

#define STM_0		/* Nothing */
#define STM_1		st %r2,8(%r15);
#define STM_2		stm %r2,%r3,8(%r15);
#define STM_3		stm %r2,%r4,8(%r15);
#define STM_4		stm %r2,%r5,8(%r15);
#define STM_5		stm %r2,%r5,8(%r15);

#define LM_0		/* Nothing */
#define LM_1		l %r2,8+96(%r15);
#define LM_2		lm %r2,%r3,8+96(%r15);
#define LM_3		lm %r2,%r4,8+96(%r15);
#define LM_4		lm %r2,%r5,8+96(%r15);
#define LM_5		lm %r2,%r5,8+96(%r15);

# ifndef __ASSEMBLER__
#  define SINGLE_THREAD_P \
  __builtin_expect (THREAD_GETMEM (THREAD_SELF,				      \
				   p_header.data.multiple_threads) == 0, 1)
# else
#  define SINGLE_THREAD_P(reg) \
	ear	reg,%a0;						      \
	l	reg,MULTIPLE_THREADS_OFFSET(reg);			      \
	ltr	reg,reg;
# endif

#elif !defined __ASSEMBLER__

/* This code should never be used but we define it anyhow.  */
# define SINGLE_THREAD_P (1)

#endif
