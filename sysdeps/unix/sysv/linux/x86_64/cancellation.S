/* Copyright (C) 2009-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2009.

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
#include <tcb-offsets.h>
#include <kernel-features.h>
#include "lowlevellock.h"

#define PTHREAD_UNWIND JUMPTARGET(__pthread_unwind)
#if IS_IN (libpthread)
# if defined SHARED && !defined NO_HIDDEN
#  undef PTHREAD_UNWIND
#  define PTHREAD_UNWIND __GI___pthread_unwind
# endif
#else
# ifndef SHARED
	.weak __pthread_unwind
# endif
#endif


#define LOAD_PRIVATE_FUTEX_WAIT(reg) \
	movl	$(FUTEX_WAIT | FUTEX_PRIVATE_FLAG), reg

/* It is crucial that the functions in this file don't modify registers
   other than %rax and %r11.  The syscall wrapper code depends on this
   because it doesn't explicitly save the other registers which hold
   relevant values.  */
	.text

	.hidden __pthread_enable_asynccancel
ENTRY(__pthread_enable_asynccancel)
	movl	%fs:CANCELHANDLING, %eax
2:	movl	%eax, %r11d
	orl	$TCB_CANCELTYPE_BITMASK, %r11d
	cmpl	%eax, %r11d
	je	1f

	lock
	cmpxchgl %r11d, %fs:CANCELHANDLING
	jnz	2b

	andl	$(TCB_CANCELSTATE_BITMASK|TCB_CANCELTYPE_BITMASK|TCB_CANCELED_BITMASK|TCB_EXITING_BITMASK|TCB_CANCEL_RESTMASK|TCB_TERMINATED_BITMASK), %r11d
	cmpl	$(TCB_CANCELTYPE_BITMASK|TCB_CANCELED_BITMASK), %r11d
	je	3f

1:	ret

3:	subq	$8, %rsp
	cfi_adjust_cfa_offset(8)
	LP_OP(mov) $TCB_PTHREAD_CANCELED, %fs:RESULT
	lock
	orl	$TCB_EXITING_BITMASK, %fs:CANCELHANDLING
	mov	%fs:CLEANUP_JMP_BUF, %RDI_LP
	call	PTHREAD_UNWIND
	hlt
END(__pthread_enable_asynccancel)


	.hidden __pthread_disable_asynccancel
ENTRY(__pthread_disable_asynccancel)
	testl	$TCB_CANCELTYPE_BITMASK, %edi
	jnz	1f

	movl	%fs:CANCELHANDLING, %eax
2:	movl	%eax, %r11d
	andl	$~TCB_CANCELTYPE_BITMASK, %r11d
	lock
	cmpxchgl %r11d, %fs:CANCELHANDLING
	jnz	2b

	movl	%r11d, %eax
3:	andl	$(TCB_CANCELING_BITMASK|TCB_CANCELED_BITMASK), %eax
	cmpl	$TCB_CANCELING_BITMASK, %eax
	je	4f
1:	ret

	/* Performance doesn't matter in this loop.  We will
	   delay until the thread is canceled.  And we will unlikely
	   enter the loop twice.  */
4:	mov	%fs:0, %RDI_LP
	movl	$__NR_futex, %eax
	xorq	%r10, %r10
	addq	$CANCELHANDLING, %rdi
	LOAD_PRIVATE_FUTEX_WAIT (%esi)
	syscall
	movl	%fs:CANCELHANDLING, %eax
	jmp	3b
END(__pthread_disable_asynccancel)
