/* `struct termios' speed frobnication functions.  Linux version.
Copyright (C) 1991, 1992, 1993, 1995, 1996 Free Software Foundation, Inc.
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

#include <stddef.h>
#include <errno.h>
#include <termios.h>


/* Return the output baud rate stored in *TERMIOS_P.  */
speed_t
cfgetospeed (termios_p)
     const struct termios *termios_p;
{
  return termios_p->c_cflag & (CBAUD | CBAUDEX);
}

/* Return the input baud rate stored in *TERMIOS_P.
   For Linux there is no difference between input and output speed.  */
strong_alias (cfgetospeed, cfgetispeed);

/* Set the output baud rate stored in *TERMIOS_P to SPEED.  */
int
cfsetospeed  (termios_p, speed)
     struct termios *termios_p;
     speed_t speed;
{
  if ((speed & ~CBAUD) != 0
      && (speed < B57600 || speed > B460800))
    {
      __set_errno (EINVAL);
      return -1;
    }

  termios_p->c_cflag &= ~(CBAUD | CBAUDEX);
  termios_p->c_cflag |= speed;

  return 0;
}

/* Set the input baud rate stored in *TERMIOS_P to SPEED.
   For Linux there is no difference between input and output speed.  */
strong_alias (cfsetospeed, cfsetispeed);
