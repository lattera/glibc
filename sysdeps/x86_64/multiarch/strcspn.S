/* Multiple versions of strcspn
   All versions must be listed in ifunc-impl-list.c.
   Copyright (C) 2009-2015 Free Software Foundation, Inc.
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

#include <config.h>

#ifdef HAVE_SSE4_SUPPORT

#include <sysdep.h>
#include <init-arch.h>

#ifdef USE_AS_STRPBRK
#define STRCSPN_SSE42	__strpbrk_sse42
#define STRCSPN_SSE2	__strpbrk_sse2
#define __GI_STRCSPN	__GI_strpbrk
#else
#ifndef STRCSPN
#define STRCSPN		strcspn
#define STRCSPN_SSE42	__strcspn_sse42
#define STRCSPN_SSE2	__strcspn_sse2
#define __GI_STRCSPN	__GI_strcspn
#endif
#endif

/* Define multiple versions only for the definition in libc.  Don't
   define multiple versions for strpbrk in static library since we
   need strpbrk before the initialization happened.  */
#if (defined SHARED || !defined USE_AS_STRPBRK) && IS_IN (libc)
	.text
ENTRY(STRCSPN)
	.type	STRCSPN, @gnu_indirect_function
	cmpl	$0, __cpu_features+KIND_OFFSET(%rip)
	jne	1f
	call	__init_cpu_features
1:	leaq	STRCSPN_SSE2(%rip), %rax
	testl	$bit_SSE4_2, __cpu_features+CPUID_OFFSET+index_SSE4_2(%rip)
	jz	2f
	leaq	STRCSPN_SSE42(%rip), %rax
2:	ret
END(STRCSPN)

# undef ENTRY
# define ENTRY(name) \
	.type STRCSPN_SSE2, @function; \
	.globl STRCSPN_SSE2; \
	.align 16; \
	STRCSPN_SSE2: cfi_startproc; \
	CALL_MCOUNT
# undef END
# define END(name) \
	cfi_endproc; .size STRCSPN_SSE2, .-STRCSPN_SSE2
# undef libc_hidden_builtin_def
/* It doesn't make sense to send libc-internal strcspn calls through a PLT.
   The speedup we get from using SSE4.2 instruction is likely eaten away
   by the indirect call in the PLT.  */
# define libc_hidden_builtin_def(name) \
	.globl __GI_STRCSPN; __GI_STRCSPN = STRCSPN_SSE2
#endif

#endif /* HAVE_SSE4_SUPPORT */

#ifdef USE_AS_STRPBRK
#include "../strpbrk.S"
#else
#include "../strcspn.S"
#endif
