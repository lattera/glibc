/* _hurd_fd_write -- write to a file descriptor; handles job control et al.
Copyright (C) 1993, 1994, 1995 Free Software Foundation, Inc.
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
#include <unistd.h>
#include <hurd.h>
#include <hurd/fd.h>

error_t
_hurd_fd_write (struct hurd_fd *fd, const void *buf, size_t *nbytes)
{
  error_t err;
  mach_msg_type_number_t wrote;

  error_t writefd (io_t port)
    {
      return __io_write (port, buf, *nbytes, -1, &wrote);
    }

  err = HURD_FD_PORT_USE (fd, _hurd_ctty_output (port, ctty, writefd));

  if (! err)
    *nbytes = wrote;

  return err;
}
