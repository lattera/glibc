/* Multiple versions of memmove.
   Copyright (C) 2010, 2011
   Free Software Foundation, Inc.
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

#include <string.h>

#ifndef NOT_IN_libc
#include <shlib-compat.h>
#include "init-arch.h"

#define MEMMOVE __memmove_sse2
#ifdef SHARED
# undef libc_hidden_builtin_def
# define libc_hidden_builtin_def(name) \
  __hidden_ver1 (__memmove_sse2, __GI_memmove, __memmove_sse2);
#endif
#endif

extern __typeof (memmove) __memmove_sse2 attribute_hidden;
extern __typeof (memmove) __memmove_ssse3 attribute_hidden;
extern __typeof (memmove) __memmove_ssse3_back attribute_hidden;

#include "string/memmove.c"

#ifndef NOT_IN_libc
libc_ifunc (memmove,
	    HAS_SSSE3
	    ? (HAS_FAST_COPY_BACKWARD
	       ? __memmove_ssse3_back : __memmove_ssse3)
	    : __memmove_sse2);

#if SHLIB_COMPAT (libc, GLIBC_2_2_5, GLIBC_2_14)
compat_symbol (libc, memmove, memcpy, GLIBC_2_2_5);
#endif
#endif
