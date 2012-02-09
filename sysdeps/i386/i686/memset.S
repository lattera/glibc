/* memset/bzero -- set memory area to CH/0
   Highly optimized version for ix86, x>=6.
   Copyright (C) 1999,2000,2003,2005,2006 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

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
ENTRY_CHK (__memset_chk)
	movl	12(%esp), %eax
	cmpl	%eax, 16(%esp)
	jb	HIDDEN_JUMPTARGET (__chk_fail)
END_CHK (__memset_chk)
#endif
ENTRY (BP_SYM (memset))
	ENTER

	cld
	pushl	%edi
	cfi_adjust_cfa_offset (4)
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
	cfi_rel_offset (edi, 0)
	andl	$3, %edx
	jz	2f	/* aligned */
	jp	3f	/* misaligned at 3, store just one byte below */
	stosb		/* misaligned at 1 or 2, store two bytes */
	decl	%ecx
	jz	1f
3:	stosb
	decl	%ecx
	jz	1f
	xorl	$1, %edx
	jnz	2f	/* was misaligned at 2 or 3, now aligned */
	stosb		/* was misaligned at 1, store third byte */
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
	cfi_adjust_cfa_offset (-4)
	cfi_restore (edi)

	LEAVE
#if BZERO_P
	ret
#else
	RET_PTR
#endif
END (BP_SYM (memset))
libc_hidden_builtin_def (memset)

#if defined PIC && !defined NOT_IN_libc && !BZERO_P
strong_alias (__memset_chk, __memset_zero_constant_len_parameter)
	.section .gnu.warning.__memset_zero_constant_len_parameter
	.string "memset used with constant zero length parameter; this could be due to transposed parameters"
#endif
