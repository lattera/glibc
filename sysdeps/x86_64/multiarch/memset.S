/* Multiple versions of memset
   All versions must be listed in ifunc-impl-list.c.
   Copyright (C) 2014-2015 Free Software Foundation, Inc.
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

#ifdef HAVE_AVX2_SUPPORT
#include <sysdep.h>
#include <shlib-compat.h>
#include <init-arch.h>

/* Define multiple versions only for the definition in lib.  */
# if IS_IN (libc)
ENTRY(memset)
	.type	memset, @gnu_indirect_function
	LOAD_RTLD_GLOBAL_RO_RDX
	leaq	__memset_sse2(%rip), %rax
	HAS_ARCH_FEATURE (AVX2_Usable)
	jz	2f
	leaq	__memset_avx2(%rip), %rax
2:	ret
END(memset)
# endif

# if IS_IN (libc)
#  undef memset
#  define memset __memset_sse2

#  undef __memset_chk
#  define __memset_chk __memset_chk_sse2

#  ifdef SHARED
#  undef libc_hidden_builtin_def
/* It doesn't make sense to send libc-internal memset calls through a PLT.
   The speedup we get from using GPR instruction is likely eaten away
   by the indirect call in the PLT.  */
#  define libc_hidden_builtin_def(name) \
	.globl __GI_memset; __GI_memset = __memset_sse2
#  endif

#  undef strong_alias
#  define strong_alias(original, alias)
# endif
#endif

#include "../memset.S"
