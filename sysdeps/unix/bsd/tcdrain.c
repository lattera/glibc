/* Copyright (C) 1991, 1996, 1997 Free Software Foundation, Inc.
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

#include "bsdtty.h"

/* Wait for pending output to be written on FD.  */
int
__libc_tcdrain (int fd)
{
  /* The TIOCSETP control waits for pending output to be written before
     affecting its changes, so we use that without changing anything.  */
  struct sgttyb b;
  if (__ioctl (fd, TIOCGETP, (void *) &b) < 0 ||
      __ioctl (fd, TIOCSETP, (void *) &b) < 0)
    return -1;
  return 0;
}
weak_alias (__libc_tcdrain, tcdrain)
