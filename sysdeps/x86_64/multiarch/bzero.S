/* Multiple versions of bzero
   Copyright (C) 2010 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <sysdep.h>
#include <init-arch.h>

	.text
ENTRY(__bzero)
	.type	__bzero, @gnu_indirect_function
	cmpl	$0, __cpu_features+KIND_OFFSET(%rip)
	jne	1f
	call	__init_cpu_features
1:	leaq	__bzero_x86_64(%rip), %rax
	testl	$bit_Prefer_SSE_for_memop, __cpu_features+FEATURE_OFFSET+index_Prefer_SSE_for_memop(%rip)
	jz	2f
	leaq	__bzero_sse2(%rip), %rax
2:	ret
END(__bzero)

	.type	__bzero_sse2, @function
__bzero_sse2:
	cfi_startproc
	CALL_MCOUNT
	mov	%rsi,%rdx	/* Adjust parameter.  */
	xorl	%esi,%esi	/* Fill with 0s.  */
	jmp	__memset_sse2
	cfi_endproc
	.size __bzero_sse2, .-__bzero_sse2

	.type	__bzero_x86_64, @function
__bzero_x86_64:
	cfi_startproc
	CALL_MCOUNT
	mov	%rsi,%rdx	/* Adjust parameter.  */
	xorl	%esi,%esi	/* Fill with 0s.  */
	jmp	__memset_x86_64
	cfi_endproc
	.size __bzero_x86_64, .-__bzero_x86_64

weak_alias (__bzero, bzero)
