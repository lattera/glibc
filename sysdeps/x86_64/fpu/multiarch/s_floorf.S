/* Copyright (C) 2011 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@gmail.come>, 2011.

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

#include <machine/asm.h>
#include <init-arch.h>


ENTRY(__floorf)
	.type	__floorf, @gnu_indirect_function
	call	__get_cpu_features@plt
	movq	%rax, %rdx
	leaq	__floorf_sse41(%rip), %rax
	testl	$bit_SSE4_1, CPUID_OFFSET+index_SSE4_1(%rdx)
	jnz	2f
	leaq	__floorf_c(%rip), %rax
2:	ret
END(__floorf)
weak_alias (__floorf, floorf)


ENTRY(__floorf_sse41)
	roundss	$1, %xmm0, %xmm0
	ret
END(__floorf_sse41)
