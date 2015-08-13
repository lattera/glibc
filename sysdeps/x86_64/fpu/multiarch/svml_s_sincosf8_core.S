/* Multiple versions of vectorized sincosf.
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
ENTRY (_ZGVdN8vvv_sincosf)
        .type   _ZGVdN8vvv_sincosf, @gnu_indirect_function
	LOAD_RTLD_GLOBAL_RO_RDX
        leaq    _ZGVdN8vvv_sincosf_avx2(%rip), %rax
	HAS_ARCH_FEATURE (AVX2_Usable)
        jz      2f
        ret
2:      leaq    _ZGVdN8vvv_sincosf_sse_wrapper(%rip), %rax
        ret
END (_ZGVdN8vvv_sincosf)
libmvec_hidden_def (_ZGVdN8vvv_sincosf)

#define _ZGVdN8vvv_sincosf _ZGVdN8vvv_sincosf_sse_wrapper
#include "../svml_s_sincosf8_core.S"
