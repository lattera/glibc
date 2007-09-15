/* Highly optimized version for i586.
   Copyright (C) 1997, 2000, 2003, 2005 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1997.

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
#include "asm-syntax.h"
#include "bp-sym.h"
#include "bp-asm.h"

/* BEWARE: `#ifdef memcpy' means that memcpy is redefined as `mempcpy',
   and the return value is the byte after the last one copied in
   the destination. */
#define MEMPCPY_P (defined memcpy)

#define PARMS	LINKAGE+8	/* space for 2 saved regs */
#define RTN	PARMS
#define DEST	RTN+RTN_SIZE
#define SRC	DEST+PTR_SIZE
#define LEN	SRC+PTR_SIZE

        .text
#if defined PIC && !defined NOT_IN_libc
ENTRY (__memcpy_chk)
	movl	12(%esp), %eax
	cmpl	%eax, 16(%esp)
	jb	HIDDEN_JUMPTARGET (__chk_fail)
END (__memcpy_chk)
#endif
ENTRY (BP_SYM (memcpy))
	ENTER

	pushl	%edi
	cfi_adjust_cfa_offset (4)
	pushl	%esi
	cfi_adjust_cfa_offset (4)

	movl	DEST(%esp), %edi
	cfi_rel_offset (edi, 4)
	movl	SRC(%esp), %esi
	cfi_rel_offset (esi, 0)
	movl	LEN(%esp), %ecx
	CHECK_BOUNDS_BOTH_WIDE (%edi, DEST(%esp), %ecx)
	CHECK_BOUNDS_BOTH_WIDE (%esi, SRC(%esp), %ecx)
	movl	%edi, %eax

	/* We need this in any case.  */
	cld

	/* Cutoff for the big loop is a size of 32 bytes since otherwise
	   the loop will never be entered.  */
	cmpl	$32, %ecx
	jbe	L(1)

	negl	%eax
	andl	$3, %eax
	subl	%eax, %ecx
	xchgl	%eax, %ecx

	rep; movsb

	movl	%eax, %ecx
	subl	$32, %ecx
	js	L(2)

	/* Read ahead to make sure we write in the cache since the stupid
	   i586 designers haven't implemented read-on-write-miss.  */
	movl	(%edi), %eax
L(3):	movl	28(%edi), %edx

	/* Now correct the loop counter.  Please note that in the following
	   code the flags are not changed anymore.  */
	subl	$32, %ecx

	movl	(%esi), %eax
	movl	4(%esi), %edx
	movl	%eax, (%edi)
	movl	%edx, 4(%edi)
	movl	8(%esi), %eax
	movl	12(%esi), %edx
	movl	%eax, 8(%edi)
	movl	%edx, 12(%edi)
	movl	16(%esi), %eax
	movl	20(%esi), %edx
	movl	%eax, 16(%edi)
	movl	%edx, 20(%edi)
	movl	24(%esi), %eax
	movl	28(%esi), %edx
	movl	%eax, 24(%edi)
	movl	%edx, 28(%edi)

	leal	32(%esi), %esi
	leal	32(%edi), %edi

	jns	L(3)

	/* Correct extra loop counter modification.  */
L(2):	addl	$32, %ecx
#if !MEMPCPY_P
	movl	DEST(%esp), %eax
#endif

L(1):	rep; movsb

#if MEMPCPY_P
	movl	%edi, %eax
#endif

	popl	%esi
	cfi_adjust_cfa_offset (-4)
	cfi_restore (esi)
	popl	%edi
	cfi_adjust_cfa_offset (-4)
	cfi_restore (edi)

	LEAVE
	RET_PTR
END (BP_SYM (memcpy))
#if !MEMPCPY_P
libc_hidden_builtin_def (memcpy)
#endif
