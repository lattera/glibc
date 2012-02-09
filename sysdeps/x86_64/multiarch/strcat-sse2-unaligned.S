/* strcat with SSE2
   Copyright (C) 2011 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef NOT_IN_libc

# include <sysdep.h>

# ifndef STRCAT
#  define STRCAT  __strcat_sse2_unaligned
# endif

# define USE_AS_STRCAT

.text
ENTRY (STRCAT)
	mov	%rdi, %r9
# ifdef USE_AS_STRNCAT
	mov	%rdx, %r8
# endif

# define RETURN  jmp L(StartStrcpyPart)
# include "strlen-sse2-pminub.S"
# undef RETURN

L(StartStrcpyPart):
	lea	(%r9, %rax), %rdi
	mov	%rsi, %rcx
	mov	%r9, %rax      /* save result */

# ifdef USE_AS_STRNCAT
	test	%r8, %r8
	jz	L(ExitZero)
#  define USE_AS_STRNCPY
# endif

# include "strcpy-sse2-unaligned.S"
#endif
