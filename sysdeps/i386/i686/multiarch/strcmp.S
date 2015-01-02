/* Multiple versions of strcmp
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

#ifdef USE_AS_STRNCMP
# define STRCMP			strncmp
# define __GI_STRCMP		__GI_strncmp
# define __STRCMP_IA32		__strncmp_ia32
# define __STRCMP_SSSE3		__strncmp_ssse3
# define __STRCMP_SSE4_2	__strncmp_sse4_2
#elif defined USE_AS_STRCASECMP_L
# define STRCMP			__strcasecmp_l
# define __GI_STRCMP		__GI_strcasecmp_l
# define __STRCMP_IA32		__strcasecmp_l_ia32
# define __STRCMP_SSSE3		__strcasecmp_l_ssse3
# define __STRCMP_SSE4_2	__strcasecmp_l_sse4_2
#elif defined USE_AS_STRNCASECMP_L
# define STRCMP			__strncasecmp_l
# define __GI_STRCMP		__GI_strncasecmp_l
# define __STRCMP_IA32		__strncasecmp_l_ia32
# define __STRCMP_SSSE3		__strncasecmp_l_ssse3
# define __STRCMP_SSE4_2	__strncasecmp_l_sse4_2
#else
# define STRCMP			strcmp
# define __GI_STRCMP		__GI_strcmp
# define __STRCMP_IA32		__strcmp_ia32
# define __STRCMP_SSSE3		__strcmp_ssse3
# define __STRCMP_SSE4_2	__strcmp_sse4_2
#endif

/* Define multiple versions only for the definition in libc.  Don't
   define multiple versions for strncmp in static library since we
   need strncmp before the initialization happened.  */
#if (defined SHARED || !defined USE_AS_STRNCMP) && IS_IN (libc)
# ifdef SHARED
	.text
ENTRY(STRCMP)
	.type	STRCMP, @gnu_indirect_function
	pushl	%ebx
	cfi_adjust_cfa_offset (4)
	cfi_rel_offset (ebx, 0)
	LOAD_PIC_REG(bx)
	cmpl	$0, KIND_OFFSET+__cpu_features@GOTOFF(%ebx)
	jne	1f
	call	__init_cpu_features
1:	leal	__STRCMP_IA32@GOTOFF(%ebx), %eax
	testl	$bit_SSSE3, CPUID_OFFSET+index_SSSE3+__cpu_features@GOTOFF(%ebx)
	jz	2f
	leal	__STRCMP_SSSE3@GOTOFF(%ebx), %eax
	testl	$bit_SSE4_2, CPUID_OFFSET+index_SSE4_2+__cpu_features@GOTOFF(%ebx)
	jz	2f
	testl	$bit_Slow_SSE4_2, FEATURE_OFFSET+index_Slow_SSE4_2+__cpu_features@GOTOFF(%ebx)
	jnz	2f
	leal	__STRCMP_SSE4_2@GOTOFF(%ebx), %eax
2:	popl	%ebx
	cfi_adjust_cfa_offset (-4)
	cfi_restore (ebx)
	ret
END(STRCMP)
# else
	.text
ENTRY(STRCMP)
	.type	STRCMP, @gnu_indirect_function
	cmpl	$0, KIND_OFFSET+__cpu_features
	jne	1f
	call	__init_cpu_features
1:	leal	__STRCMP_IA32, %eax
	testl	$bit_SSSE3, CPUID_OFFSET+index_SSSE3+__cpu_features
	jz	2f
	leal	__STRCMP_SSSE3, %eax
	testl	$bit_SSE4_2, CPUID_OFFSET+index_SSE4_2+__cpu_features
	jz	2f
	testl	$bit_Slow_SSE4_2, FEATURE_OFFSET+index_Slow_SSE4_2+__cpu_features
	jnz	2f
	leal	__STRCMP_SSE4_2, %eax
2:	ret
END(STRCMP)
# endif

# undef ENTRY
# define ENTRY(name) \
	.type __STRCMP_IA32, @function; \
	.p2align 4; \
	.globl __STRCMP_IA32; \
	.hidden __STRCMP_IA32; \
	__STRCMP_IA32: cfi_startproc; \
	CALL_MCOUNT
# undef END
# define END(name) \
	cfi_endproc; .size __STRCMP_IA32, .-__STRCMP_IA32

# ifdef SHARED
#  undef libc_hidden_builtin_def
/* IFUNC doesn't work with the hidden functions in shared library since
   they will be called without setting up EBX needed for PLT which is
   used by IFUNC.  */
#  define libc_hidden_builtin_def(name) \
	.globl __GI_STRCMP; __GI_STRCMP = __STRCMP_IA32
# endif
#endif

#if !defined USE_AS_STRNCMP && !defined USE_AS_STRCASECMP_L \
    && !defined USE_AS_STRNCASECMP_L
# include "../strcmp.S"
#endif
