/* Multiple versions of strcat
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <sysdep.h>
#include <init-arch.h>

#ifndef USE_AS_STRNCAT
# ifndef STRCAT
#  define STRCAT strcat
# endif
#endif

#ifdef USE_AS_STRNCAT
# define STRCAT_SSSE3	         	__strncat_ssse3
# define STRCAT_SSE2	            	__strncat_sse2
# define STRCAT_SSE2_UNALIGNED    	__strncat_sse2_unaligned
# define __GI_STRCAT	            	__GI_strncat
# define __GI___STRCAT              __GI___strncat
#else
# define STRCAT_SSSE3	         	__strcat_ssse3
# define STRCAT_SSE2	            	__strcat_sse2
# define STRCAT_SSE2_UNALIGNED    	__strcat_sse2_unaligned
# define __GI_STRCAT	            	__GI_strcat
# define __GI___STRCAT              __GI___strcat
#endif


/* Define multiple versions only for the definition in libc.  */
#ifndef NOT_IN_libc
	.text
ENTRY(STRCAT)
	.type	STRCAT, @gnu_indirect_function
	cmpl	$0, __cpu_features+KIND_OFFSET(%rip)
	jne	1f
	call	__init_cpu_features
1:	leaq	STRCAT_SSE2_UNALIGNED(%rip), %rax
	testl	$bit_Fast_Unaligned_Load, __cpu_features+FEATURE_OFFSET+index_Fast_Unaligned_Load(%rip)
	jnz	2f
	leaq	STRCAT_SSE2(%rip), %rax
	testl	$bit_SSSE3, __cpu_features+CPUID_OFFSET+index_SSSE3(%rip)
	jz	2f
	leaq	STRCAT_SSSE3(%rip), %rax
2:	ret
END(STRCAT)

# undef ENTRY
# define ENTRY(name) \
	.type STRCAT_SSE2, @function; \
	.align 16; \
	STRCAT_SSE2: cfi_startproc; \
	CALL_MCOUNT
# undef END
# define END(name) \
	cfi_endproc; .size STRCAT_SSE2, .-STRCAT_SSE2
# undef libc_hidden_builtin_def
/* It doesn't make sense to send libc-internal strcat calls through a PLT.
   The speedup we get from using SSSE3 instruction is likely eaten away
   by the indirect call in the PLT.  */
# define libc_hidden_builtin_def(name) \
	.globl __GI_STRCAT; __GI_STRCAT = STRCAT_SSE2
# undef libc_hidden_def
# define libc_hidden_def(name) \
	.globl __GI___STRCAT; __GI___STRCAT = STRCAT_SSE2
#endif

#ifndef USE_AS_STRNCAT
# include "../strcat.S"
#endif
