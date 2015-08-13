/* Multiple versions of vectorized log.
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
ENTRY (_ZGVbN2v_log)
        .type   _ZGVbN2v_log, @gnu_indirect_function
	LOAD_RTLD_GLOBAL_RO_RDX
        leaq    _ZGVbN2v_log_sse4(%rip), %rax
	HAS_CPU_FEATURE (SSE4_1)
        jz      2f
        ret
2:      leaq    _ZGVbN2v_log_sse2(%rip), %rax
        ret
END (_ZGVbN2v_log)
libmvec_hidden_def (_ZGVbN2v_log)

#define _ZGVbN2v_log _ZGVbN2v_log_sse2
#include "../svml_d_log2_core.S"
