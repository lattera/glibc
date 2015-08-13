/* Multiple versions of strcmp
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

#include <sysdep.h>
#include <init-arch.h>

#ifdef USE_AS_STRNCMP
/* Since the counter, %r11, is unsigned, we branch to strcmp_exitz
   if the new counter > the old one or is 0.  */
# define UPDATE_STRNCMP_COUNTER				\
	/* calculate left number to compare */		\
	lea	-16(%rcx, %r11), %r9;			\
	cmp	%r9, %r11;				\
	jb	LABEL(strcmp_exitz);			\
	test	%r9, %r9;				\
	je	LABEL(strcmp_exitz);			\
	mov	%r9, %r11

# define STRCMP_SSE42	__strncmp_sse42
# define STRCMP_SSSE3	__strncmp_ssse3
# define STRCMP_SSE2	__strncmp_sse2
# define __GI_STRCMP	__GI_strncmp
#elif defined USE_AS_STRCASECMP_L
# include "locale-defines.h"

# define UPDATE_STRNCMP_COUNTER

# define STRCMP_AVX	__strcasecmp_l_avx
# define STRCMP_SSE42	__strcasecmp_l_sse42
# define STRCMP_SSSE3	__strcasecmp_l_ssse3
# define STRCMP_SSE2	__strcasecmp_l_sse2
# define __GI_STRCMP	__GI___strcasecmp_l
#elif defined USE_AS_STRNCASECMP_L
# include "locale-defines.h"

/* Since the counter, %r11, is unsigned, we branch to strcmp_exitz
   if the new counter > the old one or is 0.  */
# define UPDATE_STRNCMP_COUNTER				\
	/* calculate left number to compare */		\
	lea	-16(%rcx, %r11), %r9;			\
	cmp	%r9, %r11;				\
	jb	LABEL(strcmp_exitz);			\
	test	%r9, %r9;				\
	je	LABEL(strcmp_exitz);			\
	mov	%r9, %r11

# define STRCMP_AVX	__strncasecmp_l_avx
# define STRCMP_SSE42	__strncasecmp_l_sse42
# define STRCMP_SSSE3	__strncasecmp_l_ssse3
# define STRCMP_SSE2	__strncasecmp_l_sse2
# define __GI_STRCMP	__GI___strncasecmp_l
#else
# define USE_AS_STRCMP
# define UPDATE_STRNCMP_COUNTER
# ifndef STRCMP
#  define STRCMP	strcmp
#  define STRCMP_SSE42	__strcmp_sse42
#  define STRCMP_SSSE3	__strcmp_ssse3
#  define STRCMP_SSE2	__strcmp_sse2
#  define __GI_STRCMP	__GI_strcmp
# endif
#endif

/* Define multiple versions only for the definition in libc.  Don't
   define multiple versions for strncmp in static library since we
   need strncmp before the initialization happened.  */
#if (defined SHARED || !defined USE_AS_STRNCMP) && IS_IN (libc)
	.text
ENTRY(STRCMP)
	.type	STRCMP, @gnu_indirect_function
	LOAD_RTLD_GLOBAL_RO_RDX
#ifdef USE_AS_STRCMP
	leaq	__strcmp_sse2_unaligned(%rip), %rax
	HAS_ARCH_FEATURE (Fast_Unaligned_Load)
	jnz     3f
#else
	HAS_ARCH_FEATURE (Slow_SSE4_2)
	jnz	2f
	leaq	STRCMP_SSE42(%rip), %rax
	HAS_CPU_FEATURE (SSE4_2)
	jnz	3f
#endif
2:	leaq	STRCMP_SSSE3(%rip), %rax
	HAS_CPU_FEATURE (SSSE3)
	jnz	3f
	leaq	STRCMP_SSE2(%rip), %rax
3:	ret
END(STRCMP)

# ifdef USE_AS_STRCASECMP_L
ENTRY(__strcasecmp)
	.type	__strcasecmp, @gnu_indirect_function
	LOAD_RTLD_GLOBAL_RO_RDX
#  ifdef HAVE_AVX_SUPPORT
	leaq	__strcasecmp_avx(%rip), %rax
	HAS_ARCH_FEATURE (AVX_Usable)
	jnz	3f
#  endif
	HAS_ARCH_FEATURE (Slow_SSE4_2)
	jnz	2f
	leaq	__strcasecmp_sse42(%rip), %rax
	HAS_CPU_FEATURE (SSE4_2)
	jnz	3f
2:	leaq	__strcasecmp_ssse3(%rip), %rax
	HAS_CPU_FEATURE (SSSE3)
	jnz	3f
	leaq	__strcasecmp_sse2(%rip), %rax
3:	ret
END(__strcasecmp)
weak_alias (__strcasecmp, strcasecmp)
# endif
# ifdef USE_AS_STRNCASECMP_L
ENTRY(__strncasecmp)
	.type	__strncasecmp, @gnu_indirect_function
	LOAD_RTLD_GLOBAL_RO_RDX
#  ifdef HAVE_AVX_SUPPORT
	leaq	__strncasecmp_avx(%rip), %rax
	HAS_ARCH_FEATURE (AVX_Usable)
	jnz	3f
#  endif
	HAS_ARCH_FEATURE (Slow_SSE4_2)
	jnz	2f
	leaq	__strncasecmp_sse42(%rip), %rax
	HAS_CPU_FEATURE (SSE4_2)
	jnz	3f
2:	leaq	__strncasecmp_ssse3(%rip), %rax
	HAS_CPU_FEATURE (SSSE3)
	jnz	3f
	leaq	__strncasecmp_sse2(%rip), %rax
3:	ret
END(__strncasecmp)
weak_alias (__strncasecmp, strncasecmp)
# endif

# undef LABEL
# define LABEL(l) .L##l##_sse42
# define GLABEL(l) l##_sse42
# define SECTION sse4.2
# include "strcmp-sse42.S"


# ifdef HAVE_AVX_SUPPORT
#  if defined USE_AS_STRCASECMP_L || defined USE_AS_STRNCASECMP_L
#   define LABEL(l) .L##l##_avx
#   define GLABEL(l) l##_avx
#   define USE_AVX 1
#   undef STRCMP_SSE42
#   define STRCMP_SSE42 STRCMP_AVX
#   define SECTION avx
#   include "strcmp-sse42.S"
#  endif
# endif


# undef ENTRY
# define ENTRY(name) \
	.type STRCMP_SSE2, @function; \
	.align 16; \
	.globl STRCMP_SSE2; \
	.hidden STRCMP_SSE2; \
	STRCMP_SSE2: cfi_startproc; \
	CALL_MCOUNT
# undef END
# define END(name) \
	cfi_endproc; .size STRCMP_SSE2, .-STRCMP_SSE2

# ifdef USE_AS_STRCASECMP_L
#  define ENTRY2(name) \
	.type __strcasecmp_sse2, @function; \
	.align 16; \
	.globl __strcasecmp_sse2; \
	.hidden __strcasecmp_sse2; \
	__strcasecmp_sse2: cfi_startproc; \
	CALL_MCOUNT
#  define END2(name) \
	cfi_endproc; .size __strcasecmp_sse2, .-__strcasecmp_sse2
# endif

# ifdef USE_AS_STRNCASECMP_L
#  define ENTRY2(name) \
	.type __strncasecmp_sse2, @function; \
	.align 16; \
	.globl __strncasecmp_sse2; \
	.hidden __strncasecmp_sse2; \
	__strncasecmp_sse2: cfi_startproc; \
	CALL_MCOUNT
#  define END2(name) \
	cfi_endproc; .size __strncasecmp_sse2, .-__strncasecmp_sse2
# endif

# undef libc_hidden_builtin_def
/* It doesn't make sense to send libc-internal strcmp calls through a PLT.
   The speedup we get from using SSE4.2 instruction is likely eaten away
   by the indirect call in the PLT.  */
# define libc_hidden_builtin_def(name) \
	.globl __GI_STRCMP; __GI_STRCMP = STRCMP_SSE2
#endif

#include "../strcmp.S"
