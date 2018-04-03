/* ABI compatibility for 'fcntl' symbol in libpthread ABI.
   Copyright (C) 2018 Free Software Foundation, Inc.
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

#include <fcntl.h>
#include <stdarg.h>
#include <shlib-compat.h>

/* libpthread once had its own fcntl, though there was no apparent reason
   for it.  There is no use in having a separate symbol in libpthread, but
   the historical ABI requires it.  For static linking, there is no need to
   provide anything here--the libc version will be linked in.  For shared
   library ABI compatibility, there must be __fcntl and fcntl symbols in
   libpthread.so.  */

#if SHLIB_COMPAT (libpthread, GLIBC_2_0, GLIBC_2_28)

static int
fcntl_compat (int fd, int cmd, ...)
{
  void *arg;
  va_list ap;
  va_start (ap, cmd);
  arg = va_arg (ap, void *);
  va_end (ap);
  return __libc_fcntl (fd, cmd, arg);
}

weak_alias (fcntl_compat, fcntl_alias)
compat_symbol (libpthread, fcntl_alias, fcntl, GLIBC_2_0);

weak_alias (fcntl_compat, __fcntl_alias)
compat_symbol (libpthread, __fcntl_alias, __fcntl, GLIBC_2_0);

#endif
