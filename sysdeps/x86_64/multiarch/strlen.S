/* strlen(str) -- determine the length of the string STR.
   Copyright (C) 2009, 2010 Free Software Foundation, Inc.
   Contributed by Ulrich Drepper <drepper@redhat.com>.
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


/* Define multiple versions only for the definition in libc and for
   the DSO.  In static binaries we need strlen before the initialization
   happened.  */
#if defined SHARED && !defined NOT_IN_libc
	.text
ENTRY(strlen)
	.type	strlen, @gnu_indirect_function
	cmpl	$0, __cpu_features+KIND_OFFSET(%rip)
	jne	1f
	call	__init_cpu_features
1:	leaq	__strlen_sse2_pminub(%rip), %rax
	testl	$bit_Prefer_PMINUB_for_stringop, __cpu_features+FEATURE_OFFSET+index_Prefer_PMINUB_for_stringop(%rip)
	jnz	2f
	leaq	__strlen_sse2(%rip), %rax
	testl	$bit_SSE4_2, __cpu_features+CPUID_OFFSET+index_SSE4_2(%rip)
	jz	2f
	leaq	__strlen_sse42(%rip), %rax
	ret
2:	testl	$bit_Slow_BSF, __cpu_features+FEATURE_OFFSET+index_Slow_BSF(%rip)
	jz	3f
	leaq    __strlen_no_bsf(%rip), %rax
3:	ret
END(strlen)

# undef ENTRY
# define ENTRY(name) \
	.type __strlen_sse2, @function; \
	.align 16; \
	__strlen_sse2: cfi_startproc; \
	CALL_MCOUNT
# undef END
# define END(name) \
	cfi_endproc; .size __strlen_sse2, .-__strlen_sse2
# undef libc_hidden_builtin_def
/* It doesn't make sense to send libc-internal strlen calls through a PLT.
   The speedup we get from using SSE4.2 instruction is likely eaten away
   by the indirect call in the PLT.  */
# define libc_hidden_builtin_def(name) \
	.globl __GI_strlen; __GI_strlen = __strlen_sse2
#endif

#include "../strlen.S"
