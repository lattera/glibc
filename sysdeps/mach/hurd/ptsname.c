/* ptsname -- return the name of a pty slave given an FD to the pty master
   Copyright (C) 1999 Free Software Foundation, Inc.
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
#include <string.h>
#include <hurd.h>
#include <hurd/fd.h>
#include <hurd/term.h>


/* Return the pathname of the pseudo terminal slave associated with
   the master FD is open on, or NULL on errors.
   The returned storage is good until the next call to this function.  */
char *
ptsname (int fd)
{
  static char peername[1024];  /* XXX */
  error_t err;

  err = __ptsname_r (fd, peername, sizeof (peername));
  if (err)
    __set_errno (err);

  return err ? NULL : peername;
}


/* Store at most BUFLEN characters of the pathname of the slave pseudo
   terminal associated with the master FD is open on in BUF.
   Return 0 on success, otherwise an error number.  */
int
__ptsname_r (int fd, char *buf, size_t buflen)
{
  char peername[1024];  /* XXX */
  size_t len;
  error_t err;

  peername[0] = '\0';
  if (err = HURD_DPORT_USE (fd, __term_get_peername (port, peername)))
    return _hurd_fd_error (fd, err);

  len = strlen (peername) + 1;
  if (len > buflen)
    return ERANGE;

  memcpy (buf, peername, len);
  return 0;
}
weak_alias (__ptsname_r, ptsname_r)
