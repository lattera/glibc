/* `struct termios' speed frobnication functions.  Linux version.
Copyright (C) 1991, 1992, 1993, 1995 Free Software Foundation, Inc.
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

static const speed_t speeds[] =
  {
    0,
    50,
    75,
    110,
    134,
    150,
    200,
    300,
    600,
    1200,
    1800,
    2400,
    4800,
    9600,
    19200,
    38400,
    38400,		/* Mention this twice here is a trick.  */
    57600,
    115200,
    230400,
  };


/* Return the output baud rate stored in *TERMIOS_P.  */
speed_t
cfgetospeed (termios_p)
     const struct termios *termios_p;
{
  speed_t retval = termios_p->c_cflag & (CBAUD | CBAUDEX);

  if (retval & CBAUDEX)
    {
      retval &= ~CBAUDEX;
      retval |= CBAUD + 1;
    }

  return retval;
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
  register unsigned int i;

  if (termios_p == NULL)
    {
      errno = EINVAL;
      return -1;
    }

  /* This allows either B1200 or 1200 to work.	XXX
     Do we really want to try to support this, given that
     fetching the speed must return one or the other?  */

  for (i = 0; i < sizeof (speeds) / sizeof (speeds[0]); ++i)
    if (i == speed || speeds[i] == speed)
      {
	termios_p->c_cflag &= ~(CBAUD | CBAUDEX);
	termios_p->c_cflag |= (i & CBAUD);
	if (i & ~CBAUD)
	  termios_p->c_cflag |= CBAUDEX;
	return 0;
      }

  errno = EINVAL;
  return -1;
}

/* Set the input baud rate stored in *TERMIOS_P to SPEED.
   For Linux there is no difference between input and output speed.  */
strong_alias (cfsetospeed, cfsetispeed);
