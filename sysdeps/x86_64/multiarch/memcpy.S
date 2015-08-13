/* Multiple versions of memcpy
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
#include <shlib-compat.h>
#include <init-arch.h>

/* Define multiple versions only for the definition in lib and for
   DSO.  In static binaries we need memcpy before the initialization
   happened.  */
#if defined SHARED && IS_IN (libc)
	.text
ENTRY(__new_memcpy)
	.type	__new_memcpy, @gnu_indirect_function
	LOAD_RTLD_GLOBAL_RO_RDX
	leaq	__memcpy_avx_unaligned(%rip), %rax
	HAS_ARCH_FEATURE (AVX_Fast_Unaligned_Load)
	jz 1f
	ret
1:	leaq	__memcpy_sse2(%rip), %rax
	HAS_ARCH_FEATURE (Slow_BSF)
	jnz	2f
	leaq	__memcpy_sse2_unaligned(%rip), %rax
	ret
2:	HAS_CPU_FEATURE (SSSE3)
	jz 3f
	leaq    __memcpy_ssse3(%rip), %rax
3:	ret
END(__new_memcpy)

# undef ENTRY
# define ENTRY(name) \
	.type __memcpy_sse2, @function; \
	.globl __memcpy_sse2; \
	.hidden __memcpy_sse2; \
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
