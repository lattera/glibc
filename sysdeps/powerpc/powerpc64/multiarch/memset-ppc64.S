/* Default memset/bzero implementation for PowerPC64.
   Copyright (C) 2013-2014 Free Software Foundation, Inc.
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

#if defined SHARED && !defined NOT_IN_libc

/* Copied from bzero.S to prevent the linker from inserting a stub
   between bzero and memset.  */
ENTRY (__bzero_ppc)
        CALL_MCOUNT 3
        mr      r5,r4
        li      r4,0
        b       L(_memset)
END_GEN_TB (__bzero_ppc,TB_TOCLESS)

# undef EALIGN
# define EALIGN(name, alignt, words)				\
  .section ".text";						\
  ENTRY_2(__memset_ppc)						\
  .align ALIGNARG(alignt);					\
  EALIGN_W_##words;						\
  BODY_LABEL(__memset_ppc):					\
  cfi_startproc;						\
  LOCALENTRY(__memset_ppc)

# undef END_GEN_TB
# define END_GEN_TB(name, mask)					\
  cfi_endproc;							\
  TRACEBACK_MASK(__memset_ppc,mask)				\
  END_2(__memset_ppc)

# undef libc_hidden_builtin_def
# define libc_hidden_builtin_def(name)				\
  .globl __GI_memset; __GI_memset = __memset_ppc

/* Do not implement __bzero at powerpc64/memset.S.  */
# define NO_BZERO_IMPL
#endif

#include <sysdeps/powerpc/powerpc64/memset.S>
