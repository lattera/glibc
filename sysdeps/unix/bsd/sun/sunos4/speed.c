/* `struct termios' speed frobnication functions.  SunOS 4 version.
   Copyright (C) 1991,1992,1993,1996,1997,2002 Free Software Foundation, Inc.
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
  };


/* Return the output baud rate stored in *TERMIOS_P.  */
speed_t
cfgetospeed (termios_p)
     const struct termios *termios_p;
{
  return termios_p->c_cflag & CBAUD;
}

/* Return the input baud rate stored in *TERMIOS_P.  */
speed_t
cfgetispeed (termios_p)
     const struct termios *termios_p;
{
  return (termios_p->c_cflag & CIBAUD) >> IBSHIFT;
}

/* Set the output baud rate stored in *TERMIOS_P to SPEED.  */
int
cfsetospeed (termios_p, speed)
     struct termios *termios_p;
     speed_t speed;
{
  register unsigned int i;

  if (termios_p == NULL)
    {
      __set_errno (EINVAL);
      return -1;
    }

  /* This allows either B1200 or 1200 to work.	XXX
     Do we really want to try to support this, given that
     fetching the speed must return one or the other?  */

  for (i = 0; i < sizeof (speeds) / sizeof (speeds[0]); ++i)
    if (i == speed || speeds[i] == speed)
      {
	termios_p->c_cflag &= ~CBAUD;
	termios_p->c_cflag |= i;
	return 0;
      }

  __set_errno (EINVAL);
  return -1;
}
libc_hidden_def (cfsetospeed)

/* Set the input baud rate stored in *TERMIOS_P to SPEED.  */
int
cfsetispeed (termios_p, speed)
     struct termios *termios_p;
     speed_t speed;
{
  register unsigned int i;

  if (termios_p == NULL)
    {
      __set_errno (EINVAL);
      return -1;
    }

  /* See comment in cfsetospeed (above).  */
  for (i = 0; i < sizeof (speeds) / sizeof (speeds[0]); ++i)
    if (i == speed || speeds[i] == speed)
      {
	termios_p->c_cflag &= ~CIBAUD;
	termios_p->c_cflag |= i << IBSHIFT;
	return 0;
      }

  __set_errno (EINVAL);
  return -1;
}
libc_hidden_def (cfsetispeed)
