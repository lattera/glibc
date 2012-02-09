/* Copy memory block and return pointer to following byte.
   For Intel 80x86, x>=6.
   This file is part of the GNU C Library.
   Copyright (C) 1998,1999,2000,2002,2004,2005 Free Software Foundation, Inc.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1998.

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
#include "asm-syntax.h"
#include "bp-sym.h"
#include "bp-asm.h"

#define PARMS	LINKAGE		/* no space for saved regs */
#define RTN	PARMS
#define DEST	RTN+RTN_SIZE
#define SRC	DEST+PTR_SIZE
#define LEN	SRC+PTR_SIZE

	.text
#if defined PIC && !defined NOT_IN_libc
ENTRY_CHK (__mempcpy_chk)
	movl	12(%esp), %eax
	cmpl	%eax, 16(%esp)
	jb	HIDDEN_JUMPTARGET (__chk_fail)
END_CHK (__mempcpy_chk)
#endif
ENTRY (BP_SYM (__mempcpy))
	ENTER

	movl	LEN(%esp), %ecx
	movl	%edi, %eax
	cfi_register (edi, eax)
	movl	DEST(%esp), %edi
	CHECK_BOUNDS_BOTH_WIDE (%edi, DEST(%esp), %ecx)
	movl	%esi, %edx
	cfi_register (esi, edx)
	movl	SRC(%esp), %esi
	CHECK_BOUNDS_BOTH_WIDE (%esi, SRC(%esp), %ecx)
	cld
	shrl	$1, %ecx
	jnc	1f
	movsb
1:	shrl	$1, %ecx
	jnc	2f
	movsw
2:	rep
	movsl
	xchgl	%edi, %eax
	cfi_restore (edi)
	movl	%edx, %esi
	cfi_restore (esi)
	RETURN_BOUNDED_POINTER (DEST(%esp))

	LEAVE
	RET_PTR
END (BP_SYM (__mempcpy))
libc_hidden_def (BP_SYM (__mempcpy))
weak_alias (BP_SYM (__mempcpy), BP_SYM (mempcpy))
libc_hidden_builtin_def (mempcpy)
