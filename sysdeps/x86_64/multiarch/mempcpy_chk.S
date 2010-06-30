/* Multiple versions of __mempcpy_chk
   Copyright (C) 2010 Free Software Foundation, Inc.
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

/* Define multiple versions only for the definition in lib and for
   DSO.  There are no multiarch mempcpy functions for static binaries.
 */
#ifndef NOT_IN_libc
# ifdef SHARED
	.text
ENTRY(__mempcpy_chk)
	.type	__mempcpy_chk, @gnu_indirect_function
	cmpl	$0, KIND_OFFSET+__cpu_features(%rip)
	jne	1f
	call	__init_cpu_features
1:	leaq	__mempcpy_chk_sse2(%rip), %rax
	testl	$bit_SSSE3, __cpu_features+CPUID_OFFSET+index_SSSE3(%rip)
	jz	2f
	leaq	__mempcpy_chk_ssse3(%rip), %rax
	testl	$bit_Fast_Copy_Backward, __cpu_features+FEATURE_OFFSET+index_Fast_Copy_Backward(%rip)
	jz	2f
	leaq	__mempcpy_chk_ssse3_back(%rip), %rax
2:	ret
END(__mempcpy_chk)
# else
#  include "../mempcpy_chk.S"
# endif
#endif
