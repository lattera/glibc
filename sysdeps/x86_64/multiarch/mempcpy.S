/* Multiple versions of mempcpy
   All versions must be listed in ifunc-impl-list.c.
   Copyright (C) 2010-2015 Free Software Foundation, Inc.
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

/* Define multiple versions only for the definition in lib and for
   DSO.  In static binaries we need mempcpy before the initialization
   happened.  */
#if defined SHARED && IS_IN (libc)
ENTRY(__mempcpy)
	.type	__mempcpy, @gnu_indirect_function
	cmpl	$0, KIND_OFFSET+__cpu_features(%rip)
	jne	1f
	call	__init_cpu_features
1:	leaq	__mempcpy_sse2(%rip), %rax
	testl	$bit_SSSE3, __cpu_features+CPUID_OFFSET+index_SSSE3(%rip)
	jz	2f
	leaq	__mempcpy_ssse3(%rip), %rax
	testl	$bit_Fast_Copy_Backward, __cpu_features+FEATURE_OFFSET+index_Fast_Copy_Backward(%rip)
	jz	2f
	leaq	__mempcpy_ssse3_back(%rip), %rax
	testl	$bit_AVX_Fast_Unaligned_Load, __cpu_features+FEATURE_OFFSET+index_AVX_Fast_Unaligned_Load(%rip)
	jz	2f
	leaq	__mempcpy_avx_unaligned(%rip), %rax
2:	ret
END(__mempcpy)

# undef ENTRY
# define ENTRY(name) \
	.type __mempcpy_sse2, @function; \
	.p2align 4; \
	.globl __mempcpy_sse2; \
	.hidden __mempcpy_sse2; \
	__mempcpy_sse2: cfi_startproc; \
	CALL_MCOUNT
# undef END
# define END(name) \
	cfi_endproc; .size __mempcpy_sse2, .-__mempcpy_sse2

# undef ENTRY_CHK
# define ENTRY_CHK(name) \
	.type __mempcpy_chk_sse2, @function; \
	.globl __mempcpy_chk_sse2; \
	.p2align 4; \
	__mempcpy_chk_sse2: cfi_startproc; \
	CALL_MCOUNT
# undef END_CHK
# define END_CHK(name) \
	cfi_endproc; .size __mempcpy_chk_sse2, .-__mempcpy_chk_sse2

# undef libc_hidden_def
# undef libc_hidden_builtin_def
/* It doesn't make sense to send libc-internal mempcpy calls through a PLT.
   The speedup we get from using SSSE3 instruction is likely eaten away
   by the indirect call in the PLT.  */
# define libc_hidden_def(name) \
	.globl __GI_mempcpy; __GI_mempcpy = __mempcpy_sse2
# define libc_hidden_builtin_def(name) \
	.globl __GI___mempcpy; __GI___mempcpy = __mempcpy_sse2
#endif

#include "../mempcpy.S"
