/* Copyright (C) 2005-2016 Free Software Foundation, Inc.
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

# ifndef NO_ERROR
#  define NO_ERROR -0x1000
# endif

/* The syscall cancellation mechanism requires userspace
   assistance, the following code does roughly this:

	do arguments (read arg5 and arg6 to registers)
	setup frame

	check if there are threads, yes jump to pseudo_cancel

	unthreaded:
		syscall
		check syscall return (jump to pre_end)
		set errno
		set return to -1
		(jump to pre_end)

	pseudo_cancel:
		cenable
		syscall
		cdisable
		check syscall return (jump to pre_end)
		set errno
		set return to -1

	pre_end
		restore stack

	It is expected that 'ret' and 'END' macros will
	append an 'undo arguments' and 'return' to the
	this PSEUDO macro. */

# undef PSEUDO
# define PSEUDO(name, syscall_name, args)				\
	ENTRY (__##syscall_name##_nocancel)				\
	DOARGS_##args					ASM_LINE_SEP	\
	stwm TREG, 64(%sp)				ASM_LINE_SEP	\
	.cfi_offset TREG, 0				ASM_LINE_SEP	\
	.cfi_adjust_cfa_offset 64			ASM_LINE_SEP	\
	stw %sp, -4(%sp)				ASM_LINE_SEP	\
	.cfi_offset 30, -4				ASM_LINE_SEP	\
	stw %r19, -32(%sp)				ASM_LINE_SEP	\
	.cfi_offset 19, -32				ASM_LINE_SEP	\
	/* Save r19 */					ASM_LINE_SEP	\
	SAVE_PIC(TREG)					ASM_LINE_SEP	\
	/* Do syscall, delay loads # */			ASM_LINE_SEP	\
	ble  0x100(%sr2,%r0)				ASM_LINE_SEP	\
	ldi SYS_ify (syscall_name), %r20 /* delay */	ASM_LINE_SEP	\
	ldi NO_ERROR,%r1				ASM_LINE_SEP	\
	cmpb,>>=,n %r1,%ret0,L(pre_nc_end)		ASM_LINE_SEP	\
	/* Restore r19 from TREG */			ASM_LINE_SEP	\
	LOAD_PIC(TREG) /* delay */			ASM_LINE_SEP	\
	SYSCALL_ERROR_HANDLER				ASM_LINE_SEP	\
	/* Use TREG for temp storage */			ASM_LINE_SEP	\
	copy %ret0, TREG /* delay */			ASM_LINE_SEP	\
	/* OPTIMIZE: Don't reload r19 */		ASM_LINE_SEP	\
	/* do a -1*syscall_ret0 */			ASM_LINE_SEP	\
	sub %r0, TREG, TREG				ASM_LINE_SEP	\
	/* Store into errno location */			ASM_LINE_SEP	\
	stw TREG, 0(%sr0,%ret0)				ASM_LINE_SEP	\
	/* return -1 as error */			ASM_LINE_SEP	\
	ldi -1, %ret0					ASM_LINE_SEP	\
L(pre_nc_end):						ASM_LINE_SEP	\
	/* No need to LOAD_PIC */			ASM_LINE_SEP	\
	/* Undo frame */				ASM_LINE_SEP	\
	ldwm -64(%sp),TREG				ASM_LINE_SEP	\
	.cfi_adjust_cfa_offset -64			ASM_LINE_SEP	\
	/* Restore rp before exit */			ASM_LINE_SEP	\
	ldw -20(%sp), %rp				ASM_LINE_SEP	\
	.cfi_restore 2					ASM_LINE_SEP	\
	ret						ASM_LINE_SEP	\
	END(__##syscall_name##_nocancel)		ASM_LINE_SEP	\
	/**********************************************/ASM_LINE_SEP	\
	ENTRY (name)							\
	DOARGS_##args					ASM_LINE_SEP	\
	stwm TREG, 64(%sp)				ASM_LINE_SEP	\
	.cfi_adjust_cfa_offset 64			ASM_LINE_SEP	\
	stw %sp, -4(%sp)				ASM_LINE_SEP	\
	.cfi_offset 30, -4				ASM_LINE_SEP	\
	stw %r19, -32(%sp)				ASM_LINE_SEP	\
	.cfi_offset 19, -32				ASM_LINE_SEP	\
	/* Done setting up frame, continue... */	ASM_LINE_SEP	\
	SINGLE_THREAD_P					ASM_LINE_SEP	\
	cmpib,<>,n 0,%ret0,L(pseudo_cancel)		ASM_LINE_SEP	\
L(unthreaded):						ASM_LINE_SEP	\
	/* Save r19 */					ASM_LINE_SEP	\
	SAVE_PIC(TREG)					ASM_LINE_SEP	\
	/* Do syscall, delay loads # */			ASM_LINE_SEP	\
	ble  0x100(%sr2,%r0)				ASM_LINE_SEP	\
	ldi SYS_ify (syscall_name), %r20 /* delay */	ASM_LINE_SEP	\
	ldi NO_ERROR,%r1				ASM_LINE_SEP	\
	cmpb,>>=,n %r1,%ret0,L(pre_end)			ASM_LINE_SEP	\
	/* Restore r19 from TREG */			ASM_LINE_SEP	\
	LOAD_PIC(TREG) /* delay */			ASM_LINE_SEP	\
	SYSCALL_ERROR_HANDLER				ASM_LINE_SEP	\
	/* Use TREG for temp storage */			ASM_LINE_SEP	\
	copy %ret0, TREG /* delay */			ASM_LINE_SEP	\
	/* OPTIMIZE: Don't reload r19 */		ASM_LINE_SEP	\
	/* do a -1*syscall_ret0 */			ASM_LINE_SEP	\
	sub %r0, TREG, TREG				ASM_LINE_SEP	\
	/* Store into errno location */			ASM_LINE_SEP	\
	stw TREG, 0(%sr0,%ret0)				ASM_LINE_SEP	\
	b L(pre_end)					ASM_LINE_SEP	\
	/* return -1 as error */			ASM_LINE_SEP	\
	ldi -1, %ret0 /* delay */			ASM_LINE_SEP	\
L(pseudo_cancel):					ASM_LINE_SEP	\
	PUSHARGS_##args /* Save args */			ASM_LINE_SEP	\
	/* Save r19 into TREG */			ASM_LINE_SEP	\
	CENABLE /* FUNC CALL */				ASM_LINE_SEP	\
	SAVE_PIC(TREG) /* delay */			ASM_LINE_SEP	\
	/* restore syscall args */			ASM_LINE_SEP	\
	POPARGS_##args					ASM_LINE_SEP	\
	/* save mask from cenable (use stub rp slot) */	ASM_LINE_SEP	\
	stw %ret0, -24(%sp)				ASM_LINE_SEP	\
	/* ... SYSCALL ... */				ASM_LINE_SEP	\
	ble 0x100(%sr2,%r0)				ASM_LINE_SEP    \
	ldi SYS_ify (syscall_name), %r20 /* delay */	ASM_LINE_SEP	\
	/* ............... */				ASM_LINE_SEP	\
	LOAD_PIC(TREG)					ASM_LINE_SEP	\
	/* pass mask as arg0 to cdisable */		ASM_LINE_SEP	\
	ldw -24(%sp), %r26				ASM_LINE_SEP	\
	CDISABLE					ASM_LINE_SEP	\
	stw %ret0, -24(%sp) /* delay */			ASM_LINE_SEP	\
	/* Restore syscall return */			ASM_LINE_SEP	\
	ldw -24(%sp), %ret0				ASM_LINE_SEP	\
	/* compare error */				ASM_LINE_SEP	\
	ldi NO_ERROR,%r1				ASM_LINE_SEP	\
	/* branch if no error */			ASM_LINE_SEP	\
	cmpb,>>=,n %r1,%ret0,L(pre_end)			ASM_LINE_SEP	\
	LOAD_PIC(TREG)	/* cond. nullify */		ASM_LINE_SEP	\
	copy %ret0, TREG /* save syscall return */	ASM_LINE_SEP	\
	SYSCALL_ERROR_HANDLER				ASM_LINE_SEP	\
	/* make syscall res value positive */		ASM_LINE_SEP	\
	sub %r0, TREG, TREG	/* delay */		ASM_LINE_SEP	\
	/* No need to LOAD_PIC */			ASM_LINE_SEP	\
	/* store into errno location */			ASM_LINE_SEP	\
	stw TREG, 0(%sr0,%ret0)				ASM_LINE_SEP	\
	/* return -1 */					ASM_LINE_SEP	\
	ldi -1, %ret0					ASM_LINE_SEP	\
L(pre_end):						ASM_LINE_SEP	\
	/* No need to LOAD_PIC */			ASM_LINE_SEP	\
	/* Undo frame */				ASM_LINE_SEP	\
	ldwm -64(%sp),TREG				ASM_LINE_SEP	\
	.cfi_adjust_cfa_offset -64			ASM_LINE_SEP	\
	/* Restore rp before exit */			ASM_LINE_SEP	\
	ldw -20(%sp), %rp				ASM_LINE_SEP	\
	.cfi_restore 2					ASM_LINE_SEP

/* Save arguments into our frame */
# define PUSHARGS_0	/* nothing to do */
# define PUSHARGS_1	PUSHARGS_0 stw %r26, -36(%sr0,%sp)	ASM_LINE_SEP	\
			.cfi_offset 26, -36			ASM_LINE_SEP
# define PUSHARGS_2	PUSHARGS_1 stw %r25, -40(%sr0,%sp)	ASM_LINE_SEP	\
			.cfi_offset 25, -40			ASM_LINE_SEP
# define PUSHARGS_3	PUSHARGS_2 stw %r24, -44(%sr0,%sp)	ASM_LINE_SEP	\
			.cfi_offset 24, -44			ASM_LINE_SEP
# define PUSHARGS_4	PUSHARGS_3 stw %r23, -48(%sr0,%sp)	ASM_LINE_SEP	\
			.cfi_offset 23, -48			ASM_LINE_SEP
# define PUSHARGS_5	PUSHARGS_4 stw %r22, -52(%sr0,%sp)	ASM_LINE_SEP	\
			.cfi_offset 22, -52			ASM_LINE_SEP
# define PUSHARGS_6	PUSHARGS_5 stw %r21, -56(%sr0,%sp)	ASM_LINE_SEP	\
			.cfi_offset 21, -56			ASM_LINE_SEP

/* Bring them back from the stack */
# define POPARGS_0	/* nothing to do */
# define POPARGS_1	POPARGS_0 ldw -36(%sr0,%sp), %r26	ASM_LINE_SEP	\
			.cfi_restore 26				ASM_LINE_SEP
# define POPARGS_2	POPARGS_1 ldw -40(%sr0,%sp), %r25	ASM_LINE_SEP	\
			.cfi_restore 25				ASM_LINE_SEP
# define POPARGS_3	POPARGS_2 ldw -44(%sr0,%sp), %r24	ASM_LINE_SEP	\
			.cfi_restore 24				ASM_LINE_SEP
# define POPARGS_4	POPARGS_3 ldw -48(%sr0,%sp), %r23	ASM_LINE_SEP	\
			.cfi_restore 23				ASM_LINE_SEP
# define POPARGS_5	POPARGS_4 ldw -52(%sr0,%sp), %r22	ASM_LINE_SEP	\
			.cfi_restore 22				ASM_LINE_SEP
# define POPARGS_6	POPARGS_5 ldw -56(%sr0,%sp), %r21	ASM_LINE_SEP	\
			.cfi_restore 21				ASM_LINE_SEP

# if IS_IN (libpthread)
#  ifdef PIC
#   define CENABLE .import __pthread_enable_asynccancel,code ASM_LINE_SEP \
			bl __pthread_enable_asynccancel,%r2 ASM_LINE_SEP
#   define CDISABLE .import __pthread_disable_asynccancel,code ASM_LINE_SEP \
			bl __pthread_disable_asynccancel,%r2 ASM_LINE_SEP
#  else
#   define CENABLE .import __pthread_enable_asynccancel,code ASM_LINE_SEP \
			bl __pthread_enable_asynccancel,%r2 ASM_LINE_SEP
#   define CDISABLE .import __pthread_disable_asynccancel,code ASM_LINE_SEP \
			bl __pthread_disable_asynccancel,%r2 ASM_LINE_SEP
#  endif
# elif IS_IN (libc)
#  ifdef PIC
#   define CENABLE .import __libc_enable_asynccancel,code ASM_LINE_SEP \
			bl __libc_enable_asynccancel,%r2 ASM_LINE_SEP
#   define CDISABLE	.import __libc_disable_asynccancel,code ASM_LINE_SEP \
			bl __libc_disable_asynccancel,%r2 ASM_LINE_SEP
#  else
#   define CENABLE .import __libc_enable_asynccancel,code ASM_LINE_SEP \
			bl __libc_enable_asynccancel,%r2 ASM_LINE_SEP
#   define CDISABLE	.import __libc_disable_asynccancel,code ASM_LINE_SEP \
			bl __libc_disable_asynccancel,%r2 ASM_LINE_SEP
#  endif
# elif IS_IN (librt)
#  ifdef PIC
#   define CENABLE .import __librt_enable_asynccancel,code ASM_LINE_SEP \
			bl __librt_enable_asynccancel,%r2 ASM_LINE_SEP
#   define CDISABLE .import __librt_disable_asynccancel,code ASM_LINE_SEP \
			bl __librt_disable_asynccancel,%r2 ASM_LINE_SEP
#  else
#   define CENABLE .import __librt_enable_asynccancel,code ASM_LINE_SEP \
			bl __librt_enable_asynccancel,%r2 ASM_LINE_SEP
#   define CDISABLE .import __librt_disable_asynccancel,code ASM_LINE_SEP \
			bl __librt_disable_asynccancel,%r2 ASM_LINE_SEP
#  endif
# else
#  error Unsupported library
# endif

# if IS_IN (libpthread)
#  define __local_multiple_threads __pthread_multiple_threads
# elif IS_IN (libc)
#  define __local_multiple_threads __libc_multiple_threads
# elif IS_IN (librt)
#  define __local_multiple_threads __librt_multiple_threads
# else
#  error Unsupported library
# endif

# ifndef __ASSEMBLER__
#  define SINGLE_THREAD_P \
  __builtin_expect (THREAD_GETMEM (THREAD_SELF, \
				   header.multiple_threads) == 0, 1)
# else
/* Read the value of header.multiple_threads from the thread pointer */
#  define SINGLE_THREAD_P							\
	mfctl %cr27, %ret0					ASM_LINE_SEP	\
	ldw MULTIPLE_THREADS_THREAD_OFFSET(%sr0,%ret0),%ret0	ASM_LINE_SEP
# endif
#elif !defined __ASSEMBLER__

/* This code should never be used but we define it anyhow.  */
# define SINGLE_THREAD_P (1)
# define NO_CANCELLATION 1

#endif
/* IS_IN (libc) || IS_IN (libpthread) || IS_IN (librt) */

#ifndef __ASSEMBLER__
# define RTLD_SINGLE_THREAD_P \
  __builtin_expect (THREAD_GETMEM (THREAD_SELF, \
				   header.multiple_threads) == 0, 1)
#endif
