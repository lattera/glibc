/* Multiple versions of memset
   All versions must be listed in ifunc-impl-list.c.
   Copyright (C) 2010-2012 Free Software Foundation, Inc.
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

/* Define multiple versions only for the definition in lib.  */
#ifndef NOT_IN_libc
ENTRY(memset)
	.type	memset, @gnu_indirect_function
	cmpl	$0, __cpu_features+KIND_OFFSET(%rip)
	jne	1f
	call	__init_cpu_features
1:	leaq	__memset_x86_64(%rip), %rax
	testl	$bit_Prefer_SSE_for_memop, __cpu_features+FEATURE_OFFSET+index_Prefer_SSE_for_memop(%rip)
	jz	2f
	leaq	__memset_sse2(%rip), %rax
2:	ret
END(memset)

/* Define internal IFUNC memset for bzero.  */
	.globl __libc_memset
	.hidden __libc_memset
	__libc_memset = memset

# define USE_SSE2 1

# undef ENTRY
# define ENTRY(name) \
	.type __memset_sse2, @function; \
	.globl __memset_sse2; \
	.p2align 4; \
	__memset_sse2: cfi_startproc; \
	CALL_MCOUNT
# undef END
# define END(name) \
	cfi_endproc; .size __memset_sse2, .-__memset_sse2

# undef ENTRY_CHK
# define ENTRY_CHK(name) \
	.type __memset_chk_sse2, @function; \
	.globl __memset_chk_sse2; \
	.p2align 4; \
	__memset_chk_sse2: cfi_startproc; \
	CALL_MCOUNT
# undef END_CHK
# define END_CHK(name) \
	cfi_endproc; .size __memset_chk_sse2, .-__memset_chk_sse2

# ifdef SHARED
#  undef libc_hidden_builtin_def
/* It doesn't make sense to send libc-internal memset calls through a PLT.
   The speedup we get from using GPR instruction is likely eaten away
   by the indirect call in the PLT.  */
#  define libc_hidden_builtin_def(name) \
	.globl __GI_memset; __GI_memset = __memset_sse2
# endif

# undef strong_alias
# define strong_alias(original, alias)
#endif

#include "../memset.S"
