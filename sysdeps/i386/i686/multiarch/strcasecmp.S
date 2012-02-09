/* Entry point for multi-version x86 strcasecmp.
   Copyright (C) 2011-2012 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <sysdep.h>
#include <init-arch.h>

#ifdef SHARED
	.text
ENTRY(__strcasecmp)
	.type	__strcasecmp, @gnu_indirect_function
	pushl	%ebx
	cfi_adjust_cfa_offset (4)
	cfi_rel_offset (ebx, 0)
	LOAD_PIC_REG(bx)
	cmpl	$0, KIND_OFFSET+__cpu_features@GOTOFF(%ebx)
	jne	1f
	call	__init_cpu_features
1:	leal	__strcasecmp_ia32@GOTOFF(%ebx), %eax
	testl	$bit_SSSE3, CPUID_OFFSET+index_SSSE3+__cpu_features@GOTOFF(%ebx)
	jz	2f
	leal	__strcasecmp_ssse3@GOTOFF(%ebx), %eax
	testl	$bit_SSE4_2, CPUID_OFFSET+index_SSE4_2+__cpu_features@GOTOFF(%ebx)
	jz	2f
	leal	__strcasecmp_sse4_2@GOTOFF(%ebx), %eax
2:	popl	%ebx
	cfi_adjust_cfa_offset (-4)
	cfi_restore (ebx)
	ret
END(__strcasecmp)
#else
	.text
ENTRY(__strcasecmp)
	.type	__strcasecmp, @gnu_indirect_function
	cmpl	$0, KIND_OFFSET+__cpu_features
	jne	1f
	call	__init_cpu_features
1:	leal	__strcasecmp_ia32, %eax
	testl	$bit_SSSE3, CPUID_OFFSET+index_SSSE3+__cpu_features
	jz	2f
	leal	__strcasecmp_ssse3, %eax
#if 0
	// XXX Temporarily
	testl	$bit_SSE4_2, CPUID_OFFSET+index_SSE4_2+__cpu_features
	jz	2f
	leal	__strcasecmp_sse4_2, %eax
#endif
2:	ret
END(__strcasecmp)
#endif

weak_alias (__strcasecmp, strcasecmp)
