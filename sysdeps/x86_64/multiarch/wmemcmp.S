/* Multiple versions of wmemcmp
   Copyright (C)  2011 Free Software Foundation, Inc.
   Contributed by Intel Corporation.
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

/* Define multiple versions only for the definition in libc. */
#ifndef NOT_IN_libc
	.text
ENTRY(wmemcmp)
	.type	wmemcmp, @gnu_indirect_function
	cmpl	$0, KIND_OFFSET+__cpu_features(%rip)
	jne	1f
	call	__init_cpu_features

1:	testl	$bit_SSSE3, __cpu_features+CPUID_OFFSET+index_SSSE3(%rip)
	jnz	2f
	leaq	__wmemcmp_sse2(%rip), %rax
	ret

2:	testl	$bit_SSE4_1, __cpu_features+CPUID_OFFSET+index_SSE4_1(%rip)
	jz	3f
	leaq	__wmemcmp_sse4_1(%rip), %rax
	ret

3:	leaq	__wmemcmp_ssse3(%rip), %rax
	ret

END(wmemcmp)
#endif
