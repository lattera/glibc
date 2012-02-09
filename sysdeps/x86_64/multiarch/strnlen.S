/* multiple version of strnlen
   Copyright (C) 2011 Free Software Foundation, Inc.
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


/* Define multiple versions only for the definition in libc.  */
#ifndef NOT_IN_libc

	.text
ENTRY(__strnlen)
	.type	__strnlen, @gnu_indirect_function
	cmpl	$0, __cpu_features+KIND_OFFSET(%rip)
	jne	1f
	call	__init_cpu_features
1:	leaq	__strnlen_sse2(%rip), %rax
	testl	$bit_Slow_BSF, __cpu_features+FEATURE_OFFSET+index_Slow_BSF(%rip)
	jz	2f
	leaq	__strnlen_sse2_no_bsf(%rip), %rax
2:	ret
END(__strnlen)

# undef ENTRY
# define ENTRY(name) \
	.type __strnlen_sse2, @function; \
	.align 16; \
	__strnlen_sse2: cfi_startproc; \
	CALL_MCOUNT
# undef END
# define END(name) \
	cfi_endproc; .size __strnlen_sse2, .-__strnlen_sse2

# undef libc_hidden_def
# define libc_hidden_def(name) \
	.globl __GI_strnlen; __GI_strnlen = __strnlen_sse2
#endif

#include "../strnlen.S"
