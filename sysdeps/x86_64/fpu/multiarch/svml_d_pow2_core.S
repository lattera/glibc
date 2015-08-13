/* Multiple versions of vectorized pow.
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

#include <sysdep.h>
#include <init-arch.h>

	.text
ENTRY (_ZGVbN2vv_pow)
        .type   _ZGVbN2vv_pow, @gnu_indirect_function
	LOAD_RTLD_GLOBAL_RO_RDX
        leaq    _ZGVbN2vv_pow_sse4(%rip), %rax
	HAS_CPU_FEATURE (SSE4_1)
        jz      2f
        ret
2:      leaq    _ZGVbN2vv_pow_sse2(%rip), %rax
        ret
END (_ZGVbN2vv_pow)
libmvec_hidden_def (_ZGVbN2vv_pow)

#define _ZGVbN2vv_pow _ZGVbN2vv_pow_sse2
#include "../svml_d_pow2_core.S"
