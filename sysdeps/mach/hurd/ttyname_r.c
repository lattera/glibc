/* Copyright (C) 1994, 1995 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <hurd.h>
#include <hurd/term.h>
#include <hurd/fd.h>

/* Store at most BUFLEN characters of the pathname of the terminal FD is
   open on in BUF.  Return 0 on success, -1 otherwise.  */
int
ttyname_r (int fd, char *buf, int buflen)
{
  error_t err;
  char nodename[1024];	/* XXX */
  char *cp;
  int len;

  nodename[0] = '\0';
  if (err = HURD_DPORT_USE (fd, __term_get_nodename (port, nodename)))
    return __hurd_dfail (fd, err), -1;

  len = (int) strlen (nodename) + 1;
  if (len > buflen)
    {
      errno = EINVAL;
      return -1;
    }

  memcpy (buf, nodename, len);
  return 0;
}
