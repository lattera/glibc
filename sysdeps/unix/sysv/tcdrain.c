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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <errno.h>
#include <stddef.h>
#include <termios.h>
#include <unistd.h>
#include <sysv_termio.h>
#include <sys/ioctl.h>

/* Wait for pending output to be written on FD.  */
int
__libc_tcdrain (int fd)
{
  /* With an argument of 1, TCSBRK just waits for output to drain.  */
  return __ioctl (fd, _TCSBRK, 1);
}
weak_alias (__libc_tcdrain, tcdrain)
