/* Copyright (C) 1992 Free Software Foundation, Inc.
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

#include <ansidecl.h>
#include <errno.h>
#include <stddef.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "sysv_termio.h"

/* Suspend or restart transmission on FD.  */
int
DEFUN(tcflow, (fd, action), int fd AND int action)
{
  switch (action)
    {
    case TCOOFF:
      return __ioctl (fd, _TCXONC, 0);
    case TCOON:
      return __ioctl (fd, _TCXONC, 1);
    case TCIOFF:
      return __ioctl (fd, _TCXONC, 2);
    case TCION:
      return __ioctl (fd, _TCXONC, 3);
    default:
      errno = EINVAL;
      return -1;
    }
}
