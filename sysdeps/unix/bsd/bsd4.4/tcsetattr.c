/* Copyright (C) 1992, 1994 Free Software Foundation, Inc.
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

/* These are defined both in termbits.h and in ioctls.h.
   They should have the same values, but perhaps not written the same way.  */
#undef ECHO
#undef MDMBUF
#undef TOSTOP
#undef FLUSHO
#undef PENDIN
#undef NOFLSH
#include <sys/ioctl.h>


/* Set the state of FD to *TERMIOS_P.  */
int
DEFUN(tcsetattr, (fd, optional_actions, termios_p),
      int fd AND int optional_actions AND CONST struct termios *termios_p)
{
  struct termios myt;

  if (optional_actions & TCSASOFT)
    {
      myt = *termios_p;
      myt.c_cflag |= CIGNORE;
      termios_p = &myt;
      optional_actions &= ~TCSASOFT;
    }

  switch (optional_actions)
    {
    case TCSANOW:
      return __ioctl (fd, TIOCSETA, termios_p);

    case TCSADRAIN:
      return __ioctl (fd, TIOCSETAW, termios_p);

    default:
      return __ioctl (fd, TIOCSETAF, termios_p);
    }
}
