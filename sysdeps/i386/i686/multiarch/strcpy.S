/* Multiple versions of strcpy
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

#if !defined (USE_AS_STPCPY) && !defined (USE_AS_STRNCPY)
# ifndef STRCPY
#  define STRCPY strcpy
# endif
#endif

#ifdef USE_AS_STPCPY
# ifdef USE_AS_STRNCPY
#  define STRCPY_SSSE3	__stpncpy_ssse3
#  define STRCPY_SSE2		__stpncpy_sse2
#  define STRCPY_IA32		__stpncpy_ia32
#  define __GI_STRCPY		__GI_stpncpy
#  define __GI___STRCPY		__GI___stpncpy
# else
#  define STRCPY_SSSE3	__stpcpy_ssse3
#  define STRCPY_SSE2		__stpcpy_sse2
#  define STRCPY_IA32		__stpcpy_ia32
#  define __GI_STRCPY		__GI_stpcpy
#  define __GI___STRCPY		__GI___stpcpy
# endif
#else
# ifdef USE_AS_STRNCPY
#  define STRCPY_SSSE3	__strncpy_ssse3
#  define STRCPY_SSE2		__strncpy_sse2
#  define STRCPY_IA32		__strncpy_ia32
#  define __GI_STRCPY		__GI_strncpy
# else
#  define STRCPY_SSSE3	__strcpy_ssse3
#  define STRCPY_SSE2		__strcpy_sse2
#  define STRCPY_IA32		__strcpy_ia32
#  define __GI_STRCPY		__GI_strcpy
# endif
#endif


/* Define multiple versions only for the definition in libc.  Don't
   define multiple versions for strncpy in static library since we
   need strncpy before the initialization happened.  */
#if IS_IN (libc)

	.text
ENTRY(STRCPY)
	.type	STRCPY, @gnu_indirect_function
	LOAD_GOT_AND_RTLD_GLOBAL_RO
	LOAD_FUNC_GOT_EAX (STRCPY_IA32)
	HAS_CPU_FEATURE (SSE2)
	jz	2f
	LOAD_FUNC_GOT_EAX (STRCPY_SSE2)
	HAS_ARCH_FEATURE (Fast_Unaligned_Load)
	jnz	2f
	HAS_CPU_FEATURE (SSSE3)
	jz	2f
	LOAD_FUNC_GOT_EAX (STRCPY_SSSE3)
2:	ret
END(STRCPY)

# undef ENTRY
# define ENTRY(name) \
	.type STRCPY_IA32, @function; \
	.align 16; \
	.globl STRCPY_IA32; \
	.hidden STRCPY_IA32; \
	STRCPY_IA32: cfi_startproc; \
	CALL_MCOUNT
# undef END
# define END(name) \
	cfi_endproc; .size STRCPY_IA32, .-STRCPY_IA32

# ifdef SHARED
#  undef libc_hidden_builtin_def
/* It doesn't make sense to send libc-internal strcpy calls through a PLT.
   The speedup we get from using SSSE3 instruction is likely eaten away
   by the indirect call in the PLT.  */
#  define libc_hidden_builtin_def(name) \
	.globl __GI_STRCPY; __GI_STRCPY = STRCPY_IA32
#  undef libc_hidden_def
#  define libc_hidden_def(name) \
	.globl __GI___STRCPY; __GI___STRCPY = STRCPY_IA32

# endif
#endif

#ifdef USE_AS_STPCPY
# ifdef USE_AS_STRNCPY
#  include "../../stpncpy.S"
# else
#  include "../../i586/stpcpy.S"
# endif
#else
# ifndef USE_AS_STRNCPY
#  include "../../i586/strcpy.S"
# endif
#endif
