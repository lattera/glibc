/* Multiple versions of memchr
   Copyright (C) 2011-2012 Free Software Foundation, Inc.
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
ENTRY(__memchr)
	.type	__memchr, @gnu_indirect_function
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

	leal	__memchr_sse2@GOTOFF(%ebx), %eax
	popl	%ebx
	CFI_POP	(%ebx)
	ret

	CFI_PUSH (%ebx)

2:	leal	__memchr_ia32@GOTOFF(%ebx), %eax
	popl	%ebx
	CFI_POP	(%ebx)
	ret

	CFI_PUSH (%ebx)

3:	leal	__memchr_sse2_bsf@GOTOFF(%ebx), %eax
	popl	%ebx
	CFI_POP	(%ebx)
	ret
END(__memchr)

weak_alias(__memchr, memchr)

# undef ENTRY
# define ENTRY(name) \
	.type __memchr_ia32, @function; \
	.globl __memchr_ia32; \
	.p2align 4; \
	__memchr_ia32: cfi_startproc; \
	CALL_MCOUNT
# undef END
# define END(name) \
	cfi_endproc; .size __memchr_ia32, .-__memchr_ia32

# undef libc_hidden_builtin_def
/* IFUNC doesn't work with the hidden functions in shared library since
   they will be called without setting up EBX needed for PLT which is
   used by IFUNC.  */
# define libc_hidden_builtin_def(name) \
	.globl __GI_memchr; __GI_memchr = __memchr_ia32

#endif
#include "../../memchr.S"
