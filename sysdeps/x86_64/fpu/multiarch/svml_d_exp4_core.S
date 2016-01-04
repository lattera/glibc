/* Multiple versions of vectorized exp.
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
ENTRY (_ZGVdN4v_exp)
        .type   _ZGVdN4v_exp, @gnu_indirect_function
	LOAD_RTLD_GLOBAL_RO_RDX
        leaq    _ZGVdN4v_exp_avx2(%rip), %rax
	HAS_ARCH_FEATURE (AVX2_Usable)
        jz      2f
        ret
2:      leaq    _ZGVdN4v_exp_sse_wrapper(%rip), %rax
        ret
END (_ZGVdN4v_exp)
libmvec_hidden_def (_ZGVdN4v_exp)

#define _ZGVdN4v_exp _ZGVdN4v_exp_sse_wrapper
#include "../svml_d_exp4_core.S"
