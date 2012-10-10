/* Multiple versions of memrchr
   All versions must be listed in ifunc-impl-list.c.
   Copyright (C) 2011, 2012 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <sysdep.h>
#include <init-arch.h>

#ifndef  NOT_IN_libc
# define CFI_POP(REG) \
	cfi_adjust_cfa_offset (-4); \
	cfi_restore (REG)

# define CFI_PUSH(REG) \
	cfi_adjust_cfa_offset (4); \
	cfi_rel_offset (REG, 0)

	.text
ENTRY(__memrchr)
	.type	__memrchr, @gnu_indirect_function
	pushl	%ebx
	CFI_PUSH (%ebx)
	LOAD_PIC_REG(bx)
	cmpl	$0, KIND_OFFSET+__cpu_features@GOTOFF(%ebx)
	jne	1f
	call	__init_cpu_features

1:	testl	$bit_SSE2, CPUID_OFFSET+index_SSE2+__cpu_features@GOTOFF(%ebx)
	jz	2f
	testl	$bit_Slow_BSF, FEATURE_OFFSET+index_Slow_BSF+__cpu_features@GOTOFF(%ebx)
	jz	3f

	leal	__memrchr_sse2@GOTOFF(%ebx), %eax
	popl	%ebx
	CFI_POP	(%ebx)
	ret

	CFI_PUSH (%ebx)

2:	leal	__memrchr_ia32@GOTOFF(%ebx), %eax
	popl	%ebx
	CFI_POP	(%ebx)
	ret

	CFI_PUSH (%ebx)

3:	leal	__memrchr_sse2_bsf@GOTOFF(%ebx), %eax
	popl	%ebx
	CFI_POP	(%ebx)
	ret
END(__memrchr)

weak_alias(__memrchr, memrchr)
#endif
