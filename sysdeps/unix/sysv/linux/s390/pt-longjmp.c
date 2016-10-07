/* Copyright (C) 2014-2016 Free Software Foundation, Inc.
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

#include <nptl/pt-longjmp.c>

#if SHLIB_COMPAT (libpthread, GLIBC_2_19, GLIBC_2_20)
/* In glibc release 2.19 new versions of longjmp-functions were introduced,
   but were reverted before 2.20. Thus both versions are the same function.  */

strong_alias (longjmp_ifunc, __v2longjmp)
compat_symbol (libpthread, __v2longjmp, longjmp, GLIBC_2_19);
strong_alias (siglongjmp_ifunc, __v2siglongjmp)
compat_symbol (libpthread, __v2siglongjmp, siglongjmp, GLIBC_2_19);
#endif /* SHLIB_COMPAT (libpthread, GLIBC_2_19, GLIBC_2_20))  */
