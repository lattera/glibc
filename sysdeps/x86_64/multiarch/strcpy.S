/* Multiple versions of strcpy
   Copyright (C) 2009, 2011 Free Software Foundation, Inc.
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

#if !defined (USE_AS_STPCPY) && !defined (USE_AS_STRNCPY)
# ifndef STRCPY
#  define STRCPY strcpy
# endif
#endif

#ifdef USE_AS_STPCPY
# ifdef USE_AS_STRNCPY
#  define STRCPY_SSSE3		__stpncpy_ssse3
#  define STRCPY_SSE2		__stpncpy_sse2
#  define STRCPY_SSE2_UNALIGNED __stpncpy_sse2_unaligned
#  define __GI_STRCPY		__GI_stpncpy
#  define __GI___STRCPY		__GI___stpncpy
# else
#  define STRCPY_SSSE3		__stpcpy_ssse3
#  define STRCPY_SSE2		__stpcpy_sse2
#  define STRCPY_SSE2_UNALIGNED	__stpcpy_sse2_unaligned
#  define __GI_STRCPY		__GI_stpcpy
#  define __GI___STRCPY		__GI___stpcpy
# endif
#else
# ifdef USE_AS_STRNCPY
#  define STRCPY_SSSE3		__strncpy_ssse3
#  define STRCPY_SSE2		__strncpy_sse2
#  define STRCPY_SSE2_UNALIGNED	__strncpy_sse2_unaligned
#  define __GI_STRCPY		__GI_strncpy
# else
#  define STRCPY_SSSE3		__strcpy_ssse3
#  define STRCPY_SSE2		__strcpy_sse2
#  define STRCPY_SSE2_UNALIGNED	__strcpy_sse2_unaligned
#  define __GI_STRCPY		__GI_strcpy
# endif
#endif


/* Define multiple versions only for the definition in libc.  */
#ifndef NOT_IN_libc
	.text
ENTRY(STRCPY)
	.type	STRCPY, @gnu_indirect_function
	cmpl	$0, __cpu_features+KIND_OFFSET(%rip)
	jne	1f
	call	__init_cpu_features
1:	leaq	STRCPY_SSE2_UNALIGNED(%rip), %rax
	testl	$bit_Fast_Unaligned_Load, __cpu_features+FEATURE_OFFSET+index_Fast_Unaligned_Load(%rip)
	jnz	2f
	leaq	STRCPY_SSE2(%rip), %rax
	testl	$bit_SSSE3, __cpu_features+CPUID_OFFSET+index_SSSE3(%rip)
	jz	2f
	leaq	STRCPY_SSSE3(%rip), %rax
2:	ret
END(STRCPY)

# undef ENTRY
# define ENTRY(name) \
	.type STRCPY_SSE2, @function; \
	.align 16; \
	STRCPY_SSE2: cfi_startproc; \
	CALL_MCOUNT
# undef END
# define END(name) \
	cfi_endproc; .size STRCPY_SSE2, .-STRCPY_SSE2
# undef libc_hidden_builtin_def
/* It doesn't make sense to send libc-internal strcpy calls through a PLT.
   The speedup we get from using SSSE3 instruction is likely eaten away
   by the indirect call in the PLT.  */
# define libc_hidden_builtin_def(name) \
	.globl __GI_STRCPY; __GI_STRCPY = STRCPY_SSE2
# undef libc_hidden_def
# define libc_hidden_def(name) \
	.globl __GI___STRCPY; __GI___STRCPY = STRCPY_SSE2
#endif

#ifndef USE_AS_STRNCPY
#include "../strcpy.S"
#endif
