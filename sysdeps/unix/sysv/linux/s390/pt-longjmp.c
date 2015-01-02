/* Copyright (C) 2014-2015 Free Software Foundation, Inc.
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
   <http://www.gnu.org/licenses/>.

   Versioned copy of nptl/pt-longjmp.c modified for versioning
   the reverted jmpbuf extension.  */

#include  <shlib-compat.h>

#if defined SHARED && SHLIB_COMPAT (libpthread, GLIBC_2_19, GLIBC_2_20)
	/* we need a unique name in case of symbol versioning.  */
# define longjmp __v1longjmp
#endif /* defined SHARED && SHLIB_COMPAT (libpthread, GLIBC_2_19, GLIBC_2_20))  */

#include <nptl/pt-longjmp.c>

#if defined SHARED && SHLIB_COMPAT (libpthread, GLIBC_2_19, GLIBC_2_20)
/* In glibc release 2.19 new versions of longjmp-functions were introduced,
   but were reverted before 2.20. Thus both versions are the same function.  */

# undef longjmp

strong_alias (__v1longjmp, __v2longjmp)
versioned_symbol (libpthread, __v1longjmp, longjmp, GLIBC_2_0);
compat_symbol (libpthread, __v2longjmp, longjmp, GLIBC_2_19);

weak_alias (siglongjmp, __v1siglongjmp)
weak_alias (siglongjmp, __v2siglongjmp)
versioned_symbol (libpthread, __v1siglongjmp, siglongjmp, GLIBC_2_0);
compat_symbol (libpthread, __v2siglongjmp, siglongjmp, GLIBC_2_19);
#endif /* defined SHARED && SHLIB_COMPAT (libpthread, GLIBC_2_19, GLIBC_2_20))  */
