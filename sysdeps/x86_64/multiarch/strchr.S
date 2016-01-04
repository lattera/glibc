/* Multiple versions of strchr
   Copyright (C) 2009-2016 Free Software Foundation, Inc.
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
#if IS_IN (libc)
	.text
ENTRY(strchr)
	.type	strchr, @gnu_indirect_function
	LOAD_RTLD_GLOBAL_RO_RDX
	leaq	__strchr_sse2(%rip), %rax
2:	HAS_ARCH_FEATURE (Slow_BSF)
	jz	3f
	leaq    __strchr_sse2_no_bsf(%rip), %rax
3:	ret
END(strchr)



# undef ENTRY
# define ENTRY(name) \
	.type __strchr_sse2, @function; \
	.align 16; \
	.globl __strchr_sse2; \
	.hidden __strchr_sse2; \
	__strchr_sse2: cfi_startproc; \
	CALL_MCOUNT
# undef END
# define END(name) \
	cfi_endproc; .size __strchr_sse2, .-__strchr_sse2
# undef libc_hidden_builtin_def
/* It doesn't make sense to send libc-internal strchr calls through a PLT.
   The speedup we get from using SSE4.2 instruction is likely eaten away
   by the indirect call in the PLT.  */
# define libc_hidden_builtin_def(name) \
	.globl __GI_strchr; __GI_strchr = __strchr_sse2
#endif

#include "../strchr.S"
