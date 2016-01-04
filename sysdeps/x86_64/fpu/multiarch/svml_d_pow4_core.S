/* Multiple versions of vectorized pow.
   Copyright (C) 2014-2016 Free Software Foundation, Inc.
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

	.text
ENTRY (_ZGVdN4vv_pow)
        .type   _ZGVdN4vv_pow, @gnu_indirect_function
	LOAD_RTLD_GLOBAL_RO_RDX
        leaq    _ZGVdN4vv_pow_avx2(%rip), %rax
	HAS_ARCH_FEATURE (AVX2_Usable)
        jz      2f
        ret
2:      leaq    _ZGVdN4vv_pow_sse_wrapper(%rip), %rax
        ret
END (_ZGVdN4vv_pow)
libmvec_hidden_def (_ZGVdN4vv_pow)

#define _ZGVdN4vv_pow _ZGVdN4vv_pow_sse_wrapper
#include "../svml_d_pow4_core.S"
