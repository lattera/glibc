/* `struct termios' speed frobnication functions.  AIX version.
   Copyright (C) 2000 Free Software Foundation, Inc.
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

/* Return the output baud rate stored in *TERMIOS_P.  */
speed_t
cfgetospeed (termios_p)
     const struct termios *termios_p;
{
  return termios_p->c_cflag & 0x0000000f;
}

/* Return the input baud rate stored in *TERMIOS_P.  */
speed_t
cfgetispeed (termios_p)
     const struct termios *termios_p;
{
  return (termios_p->c_cflag & 0x000f0000) >> 16;
}

/* Set the output baud rate stored in *TERMIOS_P to SPEED.  */
int
cfsetospeed (termios_p, speed)
     struct termios *termios_p;
     speed_t speed;
{
  if (termios_p == NULL)
    {
      __set_errno (EINVAL);
      return -1;
    }

  termios_p->c_cflag &= ~0x0000000f;
  termios_p->c_cflag |= speed & 0x0000000f;
  return 0;
}

/* Set the input baud rate stored in *TERMIOS_P to SPEED.  */
int
cfsetispeed (termios_p, speed)
     struct termios *termios_p;
     speed_t speed;
{
  if (termios_p == NULL)
    {
      __set_errno (EINVAL);
      return -1;
    }

  termios_p->c_cflag &= ~0x000f0000;
  termios_p->c_cflag |= (speed << 16) & ~0x000f0000;
  return 0;
}
