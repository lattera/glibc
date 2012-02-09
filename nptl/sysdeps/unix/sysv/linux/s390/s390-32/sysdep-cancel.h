/* Copyright (C) 2003, 2004, 2006 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <sysdep.h>
#include <tls.h>
#ifndef __ASSEMBLER__
# include <nptl/pthreadP.h>
#endif

#if !defined NOT_IN_libc || defined IS_IN_libpthread || defined IS_IN_librt

# undef PSEUDO
# define PSEUDO(name, syscall_name, args)				      \
	.text;								      \
L(pseudo_cancel):							      \
	cfi_startproc;							      \
	STM_##args							      \
	stm	%r12,%r15,48(%r15);					      \
	cfi_offset (%r15, -36);						      \
	cfi_offset (%r14, -40);						      \
	cfi_offset (%r13, -44);						      \
	cfi_offset (%r12, -48);						      \
	lr	%r14,%r15;						      \
	ahi	%r15,-96;						      \
	cfi_adjust_cfa_offset (96);					      \
	st	%r14,0(%r15);						      \
	basr    %r13,0;							      \
0:	l	%r1,1f-0b(%r13);					      \
	bas	%r14,0(%r1,%r13);					      \
	lr	%r0,%r2;						      \
	LM_##args							      \
	.if SYS_ify (syscall_name) < 256;				      \
	svc SYS_ify (syscall_name);					      \
	.else;								      \
	lhi %r1,SYS_ify (syscall_name);					      \
	svc 0;								      \
	.endif;								      \
	LR7_##args							      \
	l	%r1,2f-0b(%r13);					      \
	lr	%r12,%r2;						      \
	lr	%r2,%r0;						      \
	bas	%r14,0(%r1,%r13);					      \
	lr	%r2,%r12;						      \
	lm	%r12,%r15,48+96(%r15);					      \
	cfi_endproc;							      \
	j	L(pseudo_check);					      \
1:	.long	CENABLE-0b;						      \
2:	.long	CDISABLE-0b;						      \
ENTRY(name)								      \
	SINGLE_THREAD_P(%r1)						      \
	jne	L(pseudo_cancel);					      \
.type	__##syscall_name##_nocancel,@function;				      \
.globl	__##syscall_name##_nocancel;					      \
__##syscall_name##_nocancel:						      \
	DO_CALL(syscall_name, args);					      \
L(pseudo_check):							      \
	lhi	%r4,-4095;						      \
	clr	%r2,%r4;						      \
	jnl	SYSCALL_ERROR_LABEL;					      \
.size	__##syscall_name##_nocancel,.-__##syscall_name##_nocancel;	      \
L(pseudo_end):

# ifdef IS_IN_libpthread
#  define CENABLE	__pthread_enable_asynccancel
#  define CDISABLE	__pthread_disable_asynccancel
# elif !defined NOT_IN_libc
#  define CENABLE	__libc_enable_asynccancel
#  define CDISABLE	__libc_disable_asynccancel
# elif defined IS_IN_librt
#  define CENABLE	__librt_enable_asynccancel
#  define CDISABLE	__librt_disable_asynccancel
# else
#  error Unsupported library
# endif

#define STM_0		/* Nothing */
#define STM_1		st %r2,8(%r15);
#define STM_2		stm %r2,%r3,8(%r15);
#define STM_3		stm %r2,%r4,8(%r15);
#define STM_4		stm %r2,%r5,8(%r15);
#define STM_5		stm %r2,%r5,8(%r15);
#define STM_6		stm %r2,%r7,8(%r15);

#define LM_0		/* Nothing */
#define LM_1		l %r2,8+96(%r15);
#define LM_2		lm %r2,%r3,8+96(%r15);
#define LM_3		lm %r2,%r4,8+96(%r15);
#define LM_4		lm %r2,%r5,8+96(%r15);
#define LM_5		lm %r2,%r5,8+96(%r15);
#define LM_6		lm %r2,%r5,8+96(%r15); \
			cfi_offset (%r7, -68); \
			l %r7,96+96(%r15);

#define LR7_0		/* Nothing */
#define LR7_1		/* Nothing */
#define LR7_2		/* Nothing */
#define LR7_3		/* Nothing */
#define LR7_4		/* Nothing */
#define LR7_5		/* Nothing */
#define LR7_6		l %r7,28+96(%r15); \
			cfi_restore (%r7);

# ifndef __ASSEMBLER__
#  define SINGLE_THREAD_P \
  __builtin_expect (THREAD_GETMEM (THREAD_SELF,				      \
				   header.multiple_threads) == 0, 1)
# else
#  define SINGLE_THREAD_P(reg) \
	ear	reg,%a0;						      \
	icm	reg,15,MULTIPLE_THREADS_OFFSET(reg);
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
