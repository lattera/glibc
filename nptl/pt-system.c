/* ABI compatibility for 'system' symbol in libpthread ABI.
   Copyright (C) 2002-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2002.

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

#include <stdlib.h>
#include <shlib-compat.h>

/* libpthread once had its own 'system', though there was no apparent
   reason for it.  There is no use in having a separate symbol in
   libpthread, but the historical ABI requires it.  For static linking,
   there is no need to provide anything here--the libc version will be
   linked in.  For shared library ABI compatibility, there must be a
   'system' symbol in libpthread.so; so we define it using IFUNC to
   redirect to the libc function.  */

#if SHLIB_COMPAT (libpthread, GLIBC_2_0, GLIBC_2_22)

# if HAVE_IFUNC

extern __typeof(system) system_ifunc;
#  undef INIT_ARCH
#  define INIT_ARCH()
libc_ifunc (system_ifunc, &__libc_system)

# else  /* !HAVE_IFUNC */

static int __attribute__ ((used))
system_compat (const char *line)
{
  return __libc_system (line);
}
strong_alias (system_compat, system_ifunc)

# endif  /* HAVE_IFUNC */

compat_symbol (libpthread, system_ifunc, system, GLIBC_2_0);

#endif
