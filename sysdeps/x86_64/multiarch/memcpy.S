/* Multiple versions of memcpy
   Copyright (C) 2010, 2011
   Free Software Foundation, Inc.
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
#include <shlib-compat.h>
#include <init-arch.h>

/* Define multiple versions only for the definition in lib and for
   DSO.  In static binaries we need memcpy before the initialization
   happened.  */
#if defined SHARED && !defined NOT_IN_libc
	.text
ENTRY(__new_memcpy)
	.type	__new_memcpy, @gnu_indirect_function
	cmpl	$0, KIND_OFFSET+__cpu_features(%rip)
	jne	1f
	call	__init_cpu_features
1:	leaq	__memcpy_sse2(%rip), %rax
	testl	$bit_SSSE3, __cpu_features+CPUID_OFFSET+index_SSSE3(%rip)
	jz	2f
	leaq	__memcpy_ssse3(%rip), %rax
	testl	$bit_Fast_Copy_Backward, __cpu_features+FEATURE_OFFSET+index_Fast_Copy_Backward(%rip)
	jz	2f
	leaq	__memcpy_ssse3_back(%rip), %rax
2:	ret
END(__new_memcpy)

# undef ENTRY
# define ENTRY(name) \
	.type __memcpy_sse2, @function; \
	.p2align 4; \
	__memcpy_sse2: cfi_startproc; \
	CALL_MCOUNT
# undef END
# define END(name) \
	cfi_endproc; .size __memcpy_sse2, .-__memcpy_sse2

# undef ENTRY_CHK
# define ENTRY_CHK(name) \
	.type __memcpy_chk_sse2, @function; \
	.globl __memcpy_chk_sse2; \
	.p2align 4; \
	__memcpy_chk_sse2: cfi_startproc; \
	CALL_MCOUNT
# undef END_CHK
# define END_CHK(name) \
	cfi_endproc; .size __memcpy_chk_sse2, .-__memcpy_chk_sse2

# undef libc_hidden_builtin_def
/* It doesn't make sense to send libc-internal memcpy calls through a PLT.
   The speedup we get from using SSSE3 instruction is likely eaten away
   by the indirect call in the PLT.  */
# define libc_hidden_builtin_def(name) \
	.globl __GI_memcpy; __GI_memcpy = __memcpy_sse2

versioned_symbol (libc, __new_memcpy, memcpy, GLIBC_2_14);
#endif

#include "../memcpy.S"
