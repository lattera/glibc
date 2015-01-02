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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include <sysdep.h>
#include <tls.h>
#ifndef __ASSEMBLER__
# include <nptl/pthreadP.h>
#endif

#if IS_IN (libc) || IS_IN (libpthread) || IS_IN (librt)

/* NOTE: We do mark syscalls with unwind annotations, for the benefit of
   cancellation; but they're really only accurate at the point of the
   syscall.  The ARM unwind directives are not rich enough without adding
   a custom personality function.  */

# undef PSEUDO
# define PSEUDO(name, syscall_name, args)				\
	.text;								\
  ENTRY (__##syscall_name##_nocancel);					\
	CFI_SECTIONS;							\
	DO_CALL (syscall_name, args);					\
	cmn	r0, $4096;						\
	PSEUDO_RET;							\
  END (__##syscall_name##_nocancel);					\
  ENTRY (name);								\
	SINGLE_THREAD_P;						\
	DOARGS_##args;							\
	bne .Lpseudo_cancel;						\
	cfi_remember_state;						\
	ldr	r7, =SYS_ify (syscall_name);				\
	swi	0x0;							\
	UNDOARGS_##args;						\
	cmn	r0, $4096;						\
	PSEUDO_RET;							\
	cfi_restore_state;						\
  .Lpseudo_cancel:							\
	.fnstart;	/* matched by the .fnend in UNDOARGS below.  */	\
	DOCARGS_##args;	/* save syscall args etc. around CENABLE.  */	\
	CENABLE;							\
	mov ip, r0;		/* put mask in safe place.  */		\
	UNDOCARGS_##args;	/* restore syscall args.  */		\
	ldr	r7, =SYS_ify (syscall_name);				\
	swi	0x0;		/* do the call.  */			\
	mov	r7, r0;		/* save syscall return value.  */	\
	mov	r0, ip;		/* get mask back.  */			\
	CDISABLE;							\
	mov	r0, r7;		/* retrieve return value.  */		\
	RESTORE_LR_##args;						\
	UNDOARGS_##args;						\
	cmn	r0, $4096

/* DOARGS pushes eight bytes on the stack for five arguments, twelve bytes for
   six arguments, and four bytes for fewer.  In order to preserve doubleword
   alignment, sometimes we must save an extra register.  */

# define RESTART_UNWIND				\
	.fnend;					\
	.fnstart;				\
	.save	{r7};				\
	.save	{lr}

# define DOCARGS_0				\
	.save {r7};				\
	push	{lr};				\
	cfi_adjust_cfa_offset (4);		\
	cfi_rel_offset (lr, 0);			\
	.save	{lr}
# define UNDOCARGS_0
# define RESTORE_LR_0				\
	pop	{lr};				\
	cfi_adjust_cfa_offset (-4);		\
	cfi_restore (lr)

# define DOCARGS_1				\
	.save	{r7};				\
	push	{r0, r1, lr};			\
	cfi_adjust_cfa_offset (12);		\
	cfi_rel_offset (lr, 8);			\
	.save	{lr};				\
	.pad	#8
# define UNDOCARGS_1				\
	ldr r0, [sp], #8;			\
	cfi_adjust_cfa_offset (-8);		\
	RESTART_UNWIND
# define RESTORE_LR_1				\
	RESTORE_LR_0

# define DOCARGS_2				\
	.save	{r7};				\
	push	{r0, r1, lr};			\
	cfi_adjust_cfa_offset (12);		\
	cfi_rel_offset (lr, 8);			\
	.save	{lr};				\
	.pad	#8
# define UNDOCARGS_2				\
	pop	{r0, r1};			\
	cfi_adjust_cfa_offset (-8);		\
	RESTART_UNWIND
# define RESTORE_LR_2				\
	RESTORE_LR_0

# define DOCARGS_3				\
	.save	{r7};				\
	push	{r0, r1, r2, r3, lr};		\
	cfi_adjust_cfa_offset (20);		\
	cfi_rel_offset (lr, 16);		\
	.save	{lr};				\
	.pad	#16
# define UNDOCARGS_3				\
	pop	{r0, r1, r2, r3};		\
	cfi_adjust_cfa_offset (-16);		\
	RESTART_UNWIND
# define RESTORE_LR_3				\
	RESTORE_LR_0

# define DOCARGS_4				\
	.save	{r7};				\
	push	{r0, r1, r2, r3, lr};		\
	cfi_adjust_cfa_offset (20);		\
	cfi_rel_offset (lr, 16);		\
	.save	{lr};				\
	.pad	#16
# define UNDOCARGS_4				\
	pop	{r0, r1, r2, r3};		\
	cfi_adjust_cfa_offset (-16);		\
	RESTART_UNWIND
# define RESTORE_LR_4				\
	RESTORE_LR_0

/* r4 is only stmfd'ed for correct stack alignment.  */
# define DOCARGS_5				\
	.save	{r4, r7};			\
	push	{r0, r1, r2, r3, r4, lr};	\
	cfi_adjust_cfa_offset (24);		\
	cfi_rel_offset (lr, 20);		\
	.save	{lr};				\
	.pad	#20
# define UNDOCARGS_5				\
	pop	{r0, r1, r2, r3};		\
	cfi_adjust_cfa_offset (-16);		\
	.fnend;					\
	.fnstart;				\
	.save	{r4, r7};			\
	.save	{lr};				\
	.pad	#4
# define RESTORE_LR_5				\
	pop	{r4, lr};			\
	cfi_adjust_cfa_offset (-8);		\
	/* r4 will be marked as restored later.  */ \
	cfi_restore (lr)

# define DOCARGS_6				\
	.save	{r4, r5, r7};			\
	push	{r0, r1, r2, r3, lr};		\
	cfi_adjust_cfa_offset (20);		\
	cfi_rel_offset (lr, 16);		\
	.save	{lr};				\
	.pad	#16
# define UNDOCARGS_6				\
	pop	{r0, r1, r2, r3};		\
	cfi_adjust_cfa_offset (-16);		\
	.fnend;					\
	.fnstart;				\
	.save	{r4, r5, r7};			\
	.save	{lr};
# define RESTORE_LR_6				\
	RESTORE_LR_0

# if IS_IN (libpthread)
#  define CENABLE	bl PLTJMP(__pthread_enable_asynccancel)
#  define CDISABLE	bl PLTJMP(__pthread_disable_asynccancel)
#  define __local_multiple_threads __pthread_multiple_threads
# elif IS_IN (libc)
#  define CENABLE	bl PLTJMP(__libc_enable_asynccancel)
#  define CDISABLE	bl PLTJMP(__libc_disable_asynccancel)
#  define __local_multiple_threads __libc_multiple_threads
# elif IS_IN (librt)
#  define CENABLE	bl PLTJMP(__librt_enable_asynccancel)
#  define CDISABLE	bl PLTJMP(__librt_disable_asynccancel)
# else
#  error Unsupported library
# endif

# if IS_IN (libpthread) || IS_IN (libc)
#  ifndef __ASSEMBLER__
extern int __local_multiple_threads attribute_hidden;
#   define SINGLE_THREAD_P __builtin_expect (__local_multiple_threads == 0, 1)
#  else
#   define SINGLE_THREAD_P						\
	LDST_PCREL(ldr, ip, ip, __local_multiple_threads);		\
	teq ip, #0
#  endif
# else
/*  There is no __local_multiple_threads for librt, so use the TCB.  */
#  ifndef __ASSEMBLER__
#   define SINGLE_THREAD_P						\
  __builtin_expect (THREAD_GETMEM (THREAD_SELF,				\
				   header.multiple_threads) == 0, 1)
#  else
#   define SINGLE_THREAD_P						\
	push	{r0, lr};						\
	cfi_adjust_cfa_offset (8);					\
	cfi_rel_offset (lr, 4);						\
	GET_TLS (lr);							\
	NEGOFF_ADJ_BASE (r0, MULTIPLE_THREADS_OFFSET);			\
	ldr	ip, NEGOFF_OFF1 (r0, MULTIPLE_THREADS_OFFSET);		\
	pop	{r0, lr};						\
	cfi_adjust_cfa_offset (-8);					\
	cfi_restore (lr);						\
	teq	ip, #0
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
