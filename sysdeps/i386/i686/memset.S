/* memset/bzero -- set memory area to CH/0
   Highly optimized version for ix86, x>=6.
   Copyright (C) 1999, 2000, 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1999.

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

/* BEWARE: `#ifdef memset' means that memset is redefined as `bzero' */
#define BZERO_P (defined memset)

#define PARMS	LINKAGE+4	/* space for 1 saved reg */
#if BZERO_P
# define DEST	PARMS
# define LEN	DEST+PTR_SIZE
#else
# define RTN	PARMS
# define DEST	RTN+RTN_SIZE
# define CHR	DEST+PTR_SIZE
# define LEN	CHR+4
#endif

        .text
#if defined PIC && !defined NOT_IN_libc && !BZERO_P
ENTRY (__memset_chk)
	movl	12(%esp), %eax
	cmpl	%eax, 16(%esp)
	jb	HIDDEN_JUMPTARGET (__chk_fail)
END (__memset_chk)
#endif
ENTRY (BP_SYM (memset))
	ENTER

	cld
	pushl	%edi
	movl	DEST(%esp), %edx
	movl	LEN(%esp), %ecx
	CHECK_BOUNDS_BOTH_WIDE (%edx, DEST(%esp), %ecx)
#if BZERO_P
	xorl	%eax, %eax	/* fill with 0 */
#else
	movzbl	CHR(%esp), %eax
#endif
	jecxz	1f
	movl	%edx, %edi
	andl	$3, %edx
	jz	2f
	jnp	3f
	stosb
	decl	%ecx
	jz	1f
3:	stosb
	decl	%ecx
	jz	1f
	xorl	$3, %edx
	jz	2f
	stosb
	decl	%ecx
2:	movl	%ecx, %edx
	shrl	$2, %ecx
	andl	$3, %edx
#if !BZERO_P
	imul	$0x01010101, %eax
#endif
	rep
	stosl
	movl	%edx, %ecx
	rep
	stosb

1:
#if !BZERO_P
	movl DEST(%esp), %eax	/* start address of destination is result */
	RETURN_BOUNDED_POINTER (DEST(%esp))
#endif
	popl	%edi

	LEAVE
#if BZERO_P
	ret
#else
	RET_PTR
#endif
END (BP_SYM (memset))
libc_hidden_builtin_def (memset)
