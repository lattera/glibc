/* Copyright (C) 1998,2002,2010 Free Software Foundation, Inc.
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

#include <errno.h>
#include <paths.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/* Static buffer for `ptsname'.  */
static char buffer[sizeof (_PATH_TTY) + 2];


/* Return the pathname of the pseudo terminal slave assoicated with
   the master FD is open on, or NULL on errors.
   The returned storage is good until the next call to this function.  */
char *
ptsname (int fd)
{
  return __ptsname_r (fd, buffer, sizeof (buffer)) != 0 ? NULL : buffer;
}


/* Store at most BUFLEN characters of the pathname of the slave pseudo
   terminal associated with the master FD is open on in BUF.
   Return 0 on success, otherwise an error number.  */
int
__ptsname_r (int fd, char *buf, size_t buflen)
{
  int save_errno = errno;
  int err;
  struct stat st;

  if (buf == NULL)
    {
      __set_errno (EINVAL);
      return EINVAL;
    }

  if (!__isatty (fd))
    /* We rely on isatty to set errno properly (i.e. EBADF or ENOTTY).  */
    return errno;

  if (buflen < strlen (_PATH_TTY) + 3)
    {
      __set_errno (ERANGE);
      return ERANGE;
    }

  err = __ttyname_r (fd, buf, buflen);
  if (err != 0)
    {
      __set_errno (err);
      return errno;
    }

  buf[sizeof (_PATH_DEV) - 1] = 't';

  if (__stat (buf, &st) < 0)
    return errno;

  __set_errno (save_errno);
  return 0;
}
weak_alias (__ptsname_r, ptsname_r)
