/* Compatibility version of pthread_self in libpthread.
   Copyright (C) 2017-2018 Free Software Foundation, Inc.
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

/* Compatibility version of pthread_self for old binaries which link
   directly against libpthread's version.  */

#include <shlib-compat.h>

#if SHLIB_COMPAT (libpthread, GLIBC_2_0, GLIBC_2_27)
# include "pthread_self.c"
compat_symbol (libpthread, pthread_self, pthread_self, GLIBC_2_0);
#endif
