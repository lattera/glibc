/* vfork ABI-compatibility entry points for libpthread.
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

#include <shlib-compat.h>

/* libpthread used to have its own vfork implementation that differed
   from libc's only in having a pointless micro-optimization.  There
   is no longer any use to having a separate copy in libpthread, but
   the historical ABI requires it.  For static linking, there is no
   need to provide anything here--the libc version will be linked in.
   For shared library ABI compatibility, there must be __vfork and
   vfork symbols in libpthread.so.  */

#if (SHLIB_COMPAT (libpthread, GLIBC_2_0, GLIBC_2_20) \
     || SHLIB_COMPAT (libpthread, GLIBC_2_1_2, GLIBC_2_20))

#include <vfork.S>

#endif

#if SHLIB_COMPAT (libpthread, GLIBC_2_0, GLIBC_2_20)
compat_symbol (libpthread, __libc_vfork, vfork, GLIBC_2_0);
#endif

#if SHLIB_COMPAT (libpthread, GLIBC_2_1_2, GLIBC_2_20)
strong_alias (__libc_vfork, __vfork_compat)
compat_symbol (libpthread, __vfork_compat, __vfork, GLIBC_2_1_2);
#endif
