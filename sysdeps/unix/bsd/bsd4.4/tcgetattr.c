/* Copyright (C) 1991, 1992, 1994, 1995, 1997 Free Software Foundation, Inc.
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

#include <stddef.h>
#include <termios.h>

/* These are defined both in <bits/termios.h> and in <bits/ioctls.h>.
   They should have the same values, but perhaps not written the same way.  */
#undef ECHO
#undef MDMBUF
#undef TOSTOP
#undef FLUSHO
#undef PENDIN
#undef NOFLSH
#include <sys/ioctl.h>

/* Put the state of FD into *TERMIOS_P.  */
int
__tcgetattr (fd, termios_p)
     int fd;
     struct termios *termios_p;
{
  return __ioctl (fd, TIOCGETA, termios_p);
}

weak_alias (__tcgetattr, tcgetattr)
