/* Copyright (C) 1992, 1996, 1997 Free Software Foundation, Inc.
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
#include <stddef.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include <sysv_termio.h>

/* Flush pending data on FD.  */
int
tcflush (fd, queue_selector)
     int fd;
     int queue_selector;
{
  switch (queue_selector)
    {
    case TCIFLUSH:
      return __ioctl (fd, _TCFLSH, 0);
    case TCOFLUSH:
      return __ioctl (fd, _TCFLSH, 1);
    case TCIOFLUSH:
      return __ioctl (fd, _TCFLSH, 2);
    default:
      __set_errno (EINVAL);
      return -1;
    }
}
