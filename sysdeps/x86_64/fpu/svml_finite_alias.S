/* These aliases added as workaround to exclude unnecessary symbol
   aliases in libmvec.so while compiler creates the vector names
   based on scalar asm name.  Corresponding discussion is at
   <https://gcc.gnu.org/ml/gcc/2015-06/msg00173.html>.
   Copyright (C) 2015-2018 Free Software Foundation, Inc.
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

#define ALIAS_IMPL(alias, target) \
ENTRY (alias); \
	jmp *target@GOTPCREL(%rip); \
END (alias)

	.text
ALIAS_IMPL (_ZGVbN2v___log_finite, _ZGVbN2v_log)
ALIAS_IMPL (_ZGVcN4v___log_finite, _ZGVcN4v_log)
ALIAS_IMPL (_ZGVdN4v___log_finite, _ZGVdN4v_log)
ALIAS_IMPL (_ZGVeN8v___log_finite, _ZGVeN8v_log)

ALIAS_IMPL (_ZGVbN4v___logf_finite, _ZGVbN4v_logf)
ALIAS_IMPL (_ZGVcN8v___logf_finite, _ZGVcN8v_logf)
ALIAS_IMPL (_ZGVdN8v___logf_finite, _ZGVdN8v_logf)
ALIAS_IMPL (_ZGVeN16v___logf_finite, _ZGVeN16v_logf)

ALIAS_IMPL (_ZGVbN2v___exp_finite, _ZGVbN2v_exp)
ALIAS_IMPL (_ZGVcN4v___exp_finite, _ZGVcN4v_exp)
ALIAS_IMPL (_ZGVdN4v___exp_finite, _ZGVdN4v_exp)
ALIAS_IMPL (_ZGVeN8v___exp_finite, _ZGVeN8v_exp)

ALIAS_IMPL (_ZGVbN4v___expf_finite, _ZGVbN4v_expf)
ALIAS_IMPL (_ZGVcN8v___expf_finite, _ZGVcN8v_expf)
ALIAS_IMPL (_ZGVdN8v___expf_finite, _ZGVdN8v_expf)
ALIAS_IMPL (_ZGVeN16v___expf_finite, _ZGVeN16v_expf)

ALIAS_IMPL (_ZGVbN2vv___pow_finite, _ZGVbN2vv_pow)
ALIAS_IMPL (_ZGVcN4vv___pow_finite, _ZGVcN4vv_pow)
ALIAS_IMPL (_ZGVdN4vv___pow_finite, _ZGVdN4vv_pow)
ALIAS_IMPL (_ZGVeN8vv___pow_finite, _ZGVeN8vv_pow)

ALIAS_IMPL (_ZGVbN4vv___powf_finite, _ZGVbN4vv_powf)
ALIAS_IMPL (_ZGVcN8vv___powf_finite, _ZGVcN8vv_powf)
ALIAS_IMPL (_ZGVdN8vv___powf_finite, _ZGVdN8vv_powf)
ALIAS_IMPL (_ZGVeN16vv___powf_finite, _ZGVeN16vv_powf)
