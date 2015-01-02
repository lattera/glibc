/* Multiple versions of strcat
   All versions must be listed in ifunc-impl-list.c.
   Copyright (C) 2011-2015 Free Software Foundation, Inc.
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

#ifndef USE_AS_STRNCAT
# ifndef STRCAT
#  define STRCAT strcat
# endif
#endif

#ifdef USE_AS_STRNCAT
# define STRCAT_SSSE3	__strncat_ssse3
# define STRCAT_SSE2		__strncat_sse2
# define STRCAT_IA32		__strncat_ia32
# define __GI_STRCAT		__GI_strncat
#else
# define STRCAT_SSSE3	__strcat_ssse3
# define STRCAT_SSE2		__strcat_sse2
# define STRCAT_IA32		__strcat_ia32
# define __GI_STRCAT		__GI_strcat
#endif


/* Define multiple versions only for the definition in libc.  Don't
   define multiple versions for strncat in static library since we
   need strncat before the initialization happened.  */
#if IS_IN (libc)

# ifdef SHARED
	.text
ENTRY(STRCAT)
	.type	STRCAT, @gnu_indirect_function
	pushl	%ebx
	cfi_adjust_cfa_offset (4)
	cfi_rel_offset (ebx, 0)
	LOAD_PIC_REG(bx)
	cmpl	$0, KIND_OFFSET+__cpu_features@GOTOFF(%ebx)
	jne	1f
	call	__init_cpu_features
1:	leal	STRCAT_IA32@GOTOFF(%ebx), %eax
	testl	$bit_SSE2, CPUID_OFFSET+index_SSE2+__cpu_features@GOTOFF(%ebx)
	jz	2f
	leal	STRCAT_SSE2@GOTOFF(%ebx), %eax
	testl	$bit_Fast_Unaligned_Load, FEATURE_OFFSET+index_Fast_Unaligned_Load+__cpu_features@GOTOFF(%ebx)
	jnz	2f
	testl	$bit_SSSE3, CPUID_OFFSET+index_SSSE3+__cpu_features@GOTOFF(%ebx)
	jz	2f
	leal	STRCAT_SSSE3@GOTOFF(%ebx), %eax
2:	popl	%ebx
	cfi_adjust_cfa_offset (-4)
	cfi_restore (ebx)
	ret
END(STRCAT)
# else

ENTRY(STRCAT)
	.type	STRCAT, @gnu_indirect_function
	cmpl	$0, KIND_OFFSET+__cpu_features
	jne	1f
	call	__init_cpu_features
1:	leal	STRCAT_IA32, %eax
	testl	$bit_SSE2, CPUID_OFFSET+index_SSE2+__cpu_features
	jz	2f
	leal	STRCAT_SSE2, %eax
	testl	$bit_Fast_Unaligned_Load, FEATURE_OFFSET+index_Fast_Unaligned_Load+__cpu_features
	jnz	2f
	testl	$bit_SSSE3, CPUID_OFFSET+index_SSSE3+__cpu_features
	jz	2f
	leal	STRCAT_SSSE3, %eax
2:	ret
END(STRCAT)

# endif

# undef ENTRY
# define ENTRY(name) \
	.type STRCAT_IA32, @function; \
	.align 16; \
	.globl STRCAT_IA32; \
	.hidden STRCAT_IA32; \
	STRCAT_IA32: cfi_startproc; \
	CALL_MCOUNT
# undef END
# define END(name) \
	cfi_endproc; .size STRCAT_IA32, .-STRCAT_IA32

# ifdef SHARED
#  undef libc_hidden_builtin_def
/* It doesn't make sense to send libc-internal strcat calls through a PLT.
   The speedup we get from using SSSE3 instruction is likely eaten away
   by the indirect call in the PLT.  */
#  define libc_hidden_builtin_def(name) \
	.globl __GI_STRCAT; __GI_STRCAT = STRCAT_IA32
#  undef libc_hidden_def
#  define libc_hidden_def(name) \
	.globl __GI___STRCAT; __GI___STRCAT = STRCAT_IA32

# endif
#endif

#ifndef USE_AS_STRNCAT
# include "../../i486/strcat.S"
#endif
