/* cancellable system calls for Linux/HPPA.
   Copyright (C) 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Carlos O'Donell <carlos@baldric.uwo.ca>, 2003.

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

#if !defined NOT_IN_libc || defined IS_IN_libpthread || defined IS_IN_librt

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
	ENTRY (name)							\
	DOARGS_##args					ASM_LINE_SEP	\
	copy TREG, %r1					ASM_LINE_SEP	\
	copy %sp, TREG					ASM_LINE_SEP	\
	stwm %r1, 64(%sp)				ASM_LINE_SEP	\
	stw %rp, -20(%sp)				ASM_LINE_SEP	\
	stw TREG, -4(%sp)				ASM_LINE_SEP	\
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
	ldo -1(%r0), %ret0 /* delay */			ASM_LINE_SEP	\
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
	ldo -1(%r0), %ret0				ASM_LINE_SEP	\
L(pre_end):						ASM_LINE_SEP	\
	/* Restore rp before exit */			ASM_LINE_SEP	\
	ldw -84(%sr0,%sp), %rp				ASM_LINE_SEP	\
	/* Undo frame */				ASM_LINE_SEP	\
	ldwm -64(%sp),TREG				ASM_LINE_SEP	\
	/* No need to LOAD_PIC */			ASM_LINE_SEP

/* Save arguments into our frame */
# define PUSHARGS_0	/* nothing to do */
# define PUSHARGS_1	PUSHARGS_0 stw %r26, -36(%sr0,%sp)	ASM_LINE_SEP
# define PUSHARGS_2	PUSHARGS_1 stw %r25, -40(%sr0,%sp)	ASM_LINE_SEP
# define PUSHARGS_3	PUSHARGS_2 stw %r24, -44(%sr0,%sp)	ASM_LINE_SEP
# define PUSHARGS_4	PUSHARGS_3 stw %r23, -48(%sr0,%sp)	ASM_LINE_SEP
# define PUSHARGS_5	PUSHARGS_4 stw %r22, -52(%sr0,%sp)	ASM_LINE_SEP 
# define PUSHARGS_6	PUSHARGS_5 stw %r21, -56(%sr0,%sp)	ASM_LINE_SEP

/* Bring them back from the stack */
# define POPARGS_0	/* nothing to do */
# define POPARGS_1	POPARGS_0 ldw -36(%sr0,%sp), %r26	ASM_LINE_SEP
# define POPARGS_2	POPARGS_1 ldw -40(%sr0,%sp), %r25	ASM_LINE_SEP
# define POPARGS_3	POPARGS_2 ldw -44(%sr0,%sp), %r24	ASM_LINE_SEP
# define POPARGS_4	POPARGS_3 ldw -48(%sr0,%sp), %r23	ASM_LINE_SEP
# define POPARGS_5	POPARGS_4 ldw -52(%sr0,%sp), %r22	ASM_LINE_SEP
# define POPARGS_6	POPARGS_5 ldw -56(%sr0,%sp), %r21	ASM_LINE_SEP

# ifdef IS_IN_libpthread
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
# elif !defined NOT_IN_libc
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
# else
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
# endif

/* p_header.multiple_threads is +12 from the pthread_descr struct start,
   We could have called __get_cr27() but we really want less overhead */
# define MULTIPLE_THREADS_OFFSET 0xC

/* cr27 has been initialized to 0x0 by kernel */
# define NO_THREAD_CR27 0x0

# ifdef IS_IN_libpthread
#  define __local_multiple_threads __pthread_multiple_threads
# elif !defined NOT_IN_libc
#  define __local_multiple_threads __libc_multiple_threads
# else
#  define __local_multiple_threads __librt_multiple_threads
# endif

# ifndef __ASSEMBLER__
#  if !defined NOT_IN_libc || defined IS_IN_libpthread
extern int __local_multiple_threads attribute_hidden;
#  else
extern int __local_multiple_threads;
#  endif
#  define SINGLE_THREAD_P __builtin_expect (__local_multiple_threads == 0, 1)
# else
/* This ALT version requires newer kernel support */
#  define SINGLE_THREAD_P_MFCTL						\
	mfctl %cr27, %ret0					ASM_LINE_SEP	\
	cmpib,= NO_THREAD_CR27,%ret0,L(stp)			ASM_LINE_SEP	\
	nop							ASM_LINE_SEP	\
	ldw MULTIPLE_THREADS_OFFSET(%sr0,%ret0),%ret0		ASM_LINE_SEP	\
L(stp):								ASM_LINE_SEP
#  ifdef PIC
/* Slower version uses GOT to get value of __local_multiple_threads */
#   define SINGLE_THREAD_P							\
	addil LT%__local_multiple_threads, %r19			ASM_LINE_SEP	\
	ldw RT%__local_multiple_threads(%sr0,%r1), %ret0	ASM_LINE_SEP	\
	ldw 0(%sr0,%ret0), %ret0 				ASM_LINE_SEP
#  else
/* Slow non-pic version using DP */
#   define SINGLE_THREAD_P								\
	addil LR%__local_multiple_threads-$global$,%r27  		ASM_LINE_SEP	\
	ldw RR%__local_multiple_threads-$global$(%sr0,%r1),%ret0	ASM_LINE_SEP
#  endif
# endif
#elif !defined __ASSEMBLER__

/* This code should never be used but we define it anyhow.  */
# define SINGLE_THREAD_P (1)

#endif
/* !defined NOT_IN_libc || defined IS_IN_libpthread */
