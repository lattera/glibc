/* Multiple versions of __mempcpy_chk
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

/* Define multiple versions only for the definition in lib and for
   DSO.  There are no multiarch mempcpy functions for static binaries.
 */
#if IS_IN (libc)
# ifdef SHARED
	.text
ENTRY(__mempcpy_chk)
	.type	__mempcpy_chk, @gnu_indirect_function
	LOAD_RTLD_GLOBAL_RO_RDX
	leaq	__mempcpy_chk_sse2(%rip), %rax
	HAS_CPU_FEATURE (SSSE3)
	jz	2f
	leaq	__mempcpy_chk_ssse3(%rip), %rax
	HAS_ARCH_FEATURE (Fast_Copy_Backward)
	jz	2f
	leaq	__mempcpy_chk_ssse3_back(%rip), %rax
	HAS_ARCH_FEATURE (AVX_Fast_Unaligned_Load)
	jz	2f
	leaq	__mempcpy_chk_avx_unaligned(%rip), %rax
2:	ret
END(__mempcpy_chk)
# else
#  include "../mempcpy_chk.S"
# endif
#endif
