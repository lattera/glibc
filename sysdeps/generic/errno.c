/* Definition of `errno' variable.  Canonical version.
   Copyright (C) 2002, 2004 Free Software Foundation, Inc.
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

#include <errno.h>
#include <tls.h>
#include <dl-sysdep.h>
#undef errno

#if RTLD_PRIVATE_ERRNO

/* Code compiled for rtld refers only to this name.  */
int rtld_errno attribute_hidden;

#elif USE___THREAD

__thread int errno;
extern __thread int __libc_errno __attribute__ ((alias ("errno")))
  attribute_hidden;

#else

/* This differs from plain `int errno;' in that it doesn't create
   a common definition, but a plain symbol that resides in .bss,
   which can have an alias.  */
int errno __attribute__ ((nocommon));
strong_alias (errno, _errno)

/* We declare these with compat_symbol so that they are not visible at
   link time.  Programs must use the accessor functions.  RTLD is special,
   since it's not exported from there at any time.  */
# if defined HAVE_ELF && defined SHARED && defined DO_VERSIONING
#  include <shlib-compat.h>
compat_symbol (libc, errno, errno, GLIBC_2_0);
compat_symbol (libc, _errno, _errno, GLIBC_2_0);
# endif

#endif
