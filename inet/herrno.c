/* Copyright (C) 1996,97,98,2002,2003 Free Software Foundation, Inc.
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

#include <features.h>
#include <netdb.h>
#undef h_errno

#include <tls.h>

/* We need to have the error status variable of the resolver
   accessible in the libc.  */

#if USE___THREAD
__thread int h_errno;
extern __thread int __libc_h_errno __attribute__ ((alias ("h_errno")))
  attribute_hidden;
# define h_errno __libc_h_errno
#else
/* This differs from plain `int h_errno;' in that it doesn't create
   a common definition, but a plain symbol that resides in .bss,
   which can have an alias.  */
int h_errno __attribute__((section (".bss")));
weak_alias (h_errno, _h_errno)

/* We declare these with compat_symbol so that they are not
   visible at link time.  Programs must use the accessor functions.  */
# if defined HAVE_ELF && defined SHARED && defined DO_VERSIONING
#  include <shlib-compat.h>
compat_symbol (libc, h_errno, h_errno, GLIBC_2_0);
compat_symbol (libc, _h_errno, _h_errno, GLIBC_2_0);
# endif
#endif
