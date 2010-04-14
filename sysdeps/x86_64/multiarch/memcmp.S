/* Multiple versions of memcmp
   Copyright (C) 2010 Free Software Foundation, Inc.
   Contributed by Intel Corporation.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <sysdep.h>
#include <init-arch.h>

/* Define multiple versions only for the definition in libc. */
#ifndef NOT_IN_libc
	.text
ENTRY(memcmp)
	.type	memcmp, @gnu_indirect_function
	cmpl	$0, KIND_OFFSET+__cpu_features(%rip)
	jne	1f
	call	__init_cpu_features
1:	leaq	__memcmp_sse2(%rip), %rax
	testl	$bit_SSE4_1, __cpu_features+CPUID_OFFSET+index_SSE4_1(%rip)
	jz	2f
	leaq	__memcmp_sse4_1(%rip), %rax
2:	ret
END(memcmp)

# undef ENTRY
# define ENTRY(name) \
	.type __memcmp_sse2, @function; \
	.p2align 4; \
	__memcmp_sse2: cfi_startproc; \
	CALL_MCOUNT
# undef END
# define END(name) \
	cfi_endproc; .size __memcmp_sse2, .-__memcmp_sse2

# ifdef SHARED
#  undef libc_hidden_builtin_def
/* IFUNC doesn't work with the hidden functions in shared library since
   they will be called without setting up EBX needed for PLT which is
   used by IFUNC.  */
#  define libc_hidden_builtin_def(name) \
	.globl __GI_memcmp; __GI_memcmp = __memcmp_sse2
# endif
#endif

#include "../memcmp.S"
