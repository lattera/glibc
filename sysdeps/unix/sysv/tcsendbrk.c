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
#include <signal.h>
#include <termios.h>
#include <unistd.h>
#include <sysv_termio.h>
#include <sys/ioctl.h>

/* Send zero bits on FD.  */
int
DEFUN(tcsendbreak, (fd, duration), int fd AND int duration)
{
  /* The break lasts 0.25 to 0.5 seconds if DURATION is zero,
     and an implementation-defined period if DURATION is nonzero.
     We define a positive DURATION to be number of milliseconds to break.  */
  if (duration <= 0)
    return __ioctl (fd, _TCSBRK, 0);

  /* ioctl can't send a break of any other duration for us.
     This could be changed to use trickery (e.g. lower speed and
     send a '\0') to send the break, but for now just return an error.  */
  errno = EINVAL;
  return -1;
}
