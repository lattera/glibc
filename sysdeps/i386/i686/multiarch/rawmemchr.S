/* Multiple versions of rawmemchr
   Copyright (C) 2011 Free Software Foundation, Inc.
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

#ifndef  NOT_IN_libc
	.section	.gnu.linkonce.t.__i686.get_pc_thunk.bx,"ax",@progbits
	.globl	__i686.get_pc_thunk.bx
	.hidden	__i686.get_pc_thunk.bx
	.p2align 4
	.type	__i686.get_pc_thunk.bx,@function
__i686.get_pc_thunk.bx:
	movl	(%esp), %ebx
	ret

# define CFI_POP(REG) \
	cfi_adjust_cfa_offset (-4); \
	cfi_restore (REG)

# define CFI_PUSH(REG) \
	cfi_adjust_cfa_offset (4); \
	cfi_rel_offset (REG, 0)

	.text
ENTRY(__rawmemchr)
	.type	__rawmemchr, @gnu_indirect_function
	pushl	%ebx
	CFI_PUSH (%ebx)
	call	__i686.get_pc_thunk.bx
	addl	$_GLOBAL_OFFSET_TABLE_, %ebx
	cmpl	$0, KIND_OFFSET+__cpu_features@GOTOFF(%ebx)
	jne	1f
	call	__init_cpu_features

1:	testl	$bit_SSE2, CPUID_OFFSET+index_SSE2+__cpu_features@GOTOFF(%ebx)
	jz	2f
	testl	$bit_Slow_BSF, FEATURE_OFFSET+index_Slow_BSF+__cpu_features@GOTOFF(%ebx)
	jz	3f

	leal	__rawmemchr_sse2@GOTOFF(%ebx), %eax
	popl	%ebx
	CFI_POP	(%ebx)
	ret

	CFI_PUSH (%ebx)

2:	leal	__rawmemchr_ia32@GOTOFF(%ebx), %eax
	popl	%ebx
	CFI_POP	(%ebx)
	ret

	CFI_PUSH (%ebx)

3:	leal	__rawmemchr_sse2_bsf@GOTOFF(%ebx), %eax
	popl	%ebx
	CFI_POP	(%ebx)
	ret
END(__rawmemchr)

weak_alias(__rawmemchr, rawmemchr)

# undef ENTRY
# define ENTRY(name) \
	.type __rawmemchr_ia32, @function; \
	.globl __rawmemchr_ia32; \
	.p2align 4; \
	__rawmemchr_ia32: cfi_startproc; \
	CALL_MCOUNT
# undef END
# define END(name) \
	cfi_endproc; .size __rawmemchr_ia32, .-__rawmemchr_ia32

# undef libc_hidden_def
/* IFUNC doesn't work with the hidden functions in shared library since
   they will be called without setting up EBX needed for PLT which is
   used by IFUNC.  */
# define libc_hidden_def(name) \
	.globl __GI___rawmemchr; __GI___rawmemchr = __rawmemchr_ia32

#endif
#include "../../rawmemchr.S"
