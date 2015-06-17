/* Multiple versions of vectorized logf.
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
ENTRY (_ZGVbN4v_logf)
        .type   _ZGVbN4v_logf, @gnu_indirect_function
        cmpl    $0, KIND_OFFSET+__cpu_features(%rip)
        jne     1f
        call    __init_cpu_features
1:      leaq    _ZGVbN4v_logf_sse4(%rip), %rax
        testl   $bit_SSE4_1, __cpu_features+CPUID_OFFSET+index_SSE4_1(%rip)
        jz      2f
        ret
2:      leaq    _ZGVbN4v_logf_sse2(%rip), %rax
        ret
END (_ZGVbN4v_logf)
libmvec_hidden_def (_ZGVbN4v_logf)

#define _ZGVbN4v_logf _ZGVbN4v_logf_sse2
#include "../svml_s_logf4_core.S"
