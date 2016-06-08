/* memset/bzero -- set memory area to CH/0
   Optimized version for x86-64.
   Copyright (C) 2002-2016 Free Software Foundation, Inc.
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

#define VEC_SIZE	16
#define VEC(i)		xmm##i
/* Don't use movups and movaps since it will get larger nop paddings for
   alignment.  */
#define VMOVU		movdqu
#define VMOVA		movdqa

#define VDUP_TO_VEC0_AND_SET_RETURN(d, r) \
  movd d, %xmm0; \
  movq r, %rax; \
  punpcklbw %xmm0, %xmm0; \
  punpcklwd %xmm0, %xmm0; \
  pshufd $0, %xmm0, %xmm0

#define SECTION(p)		p

#ifndef MEMSET_SYMBOL
# define MEMSET_CHK_SYMBOL(p,s)	p
# define MEMSET_SYMBOL(p,s)	memset
#endif

#include "multiarch/memset-vec-unaligned-erms.S"

libc_hidden_builtin_def (memset)

#if defined SHARED && IS_IN (libc) && !defined USE_MULTIARCH
strong_alias (__memset_chk, __memset_zero_constant_len_parameter)
	.section .gnu.warning.__memset_zero_constant_len_parameter
	.string "memset used with constant zero length parameter; this could be due to transposed parameters"
#endif
