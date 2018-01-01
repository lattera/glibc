/* Copyright (C) 1991-2018 Free Software Foundation, Inc.
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

#include <libintl.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

/* Return a string describing the errno code in ERRNUM.
   The storage is good only until the next call to strerror.
   Writing to the storage causes undefined behavior.  */
libc_freeres_ptr (static char *buf);

char *
strerror (int errnum)
{
  char *ret = __strerror_r (errnum, NULL, 0);
  int saved_errno;

  if (__glibc_likely (ret != NULL))
    return ret;
  saved_errno = errno;
  if (buf == NULL)
    buf = malloc (1024);
  __set_errno (saved_errno);
  if (buf == NULL)
    return _("Unknown error");
  return __strerror_r (errnum, buf, 1024);
}
