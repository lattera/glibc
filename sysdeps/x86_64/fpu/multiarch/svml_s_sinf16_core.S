/* Multiple versions of vectorized sinf.
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
ENTRY (_ZGVeN16v_sinf)
        .type   _ZGVeN16v_sinf, @gnu_indirect_function
        cmpl    $0, KIND_OFFSET+__cpu_features(%rip)
        jne     1f
        call    __init_cpu_features
1:      leaq    _ZGVeN16v_sinf_skx(%rip), %rax
        testl   $bit_AVX512DQ_Usable, __cpu_features+FEATURE_OFFSET+index_AVX512DQ_Usable(%rip)
        jnz     2f
        leaq    _ZGVeN16v_sinf_knl(%rip), %rax
        testl   $bit_AVX512F_Usable, __cpu_features+FEATURE_OFFSET+index_AVX512F_Usable(%rip)
        jnz     2f
        leaq    _ZGVeN16v_sinf_avx2_wrapper(%rip), %rax
2:      ret
END (_ZGVeN16v_sinf)

#define _ZGVeN16v_sinf _ZGVeN16v_sinf_avx2_wrapper
#include "../svml_s_sinf16_core.S"
