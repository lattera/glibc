/* Copyright (C) 1992, 1995, 1996, 1997 Free Software Foundation, Inc.
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
#include <sysv_termio.h>
#include <termios.h>
#include <sys/ioctl.h>

/* Put the state of FD into *TERMIOS_P.  */
int
__tcgetattr (fd, termios_p)
     int fd;
     struct termios *termios_p;
{
  struct __sysv_termio buf;
  int termio_speed;

  if (termios_p == NULL)
    {
      __set_errno (EINVAL);
      return -1;
    }

  if (__ioctl (fd, _TCGETA, &buf) < 0)
    return -1;

  termio_speed = buf.c_cflag & _SYSV_CBAUD;
  termios_p->__ospeed =
    (termio_speed == _SYSV_B0 ? 0 :
     termio_speed == _SYSV_B50 ? 50 :
     termio_speed == _SYSV_B75 ? 75 :
     termio_speed == _SYSV_B110 ? 110 :
     termio_speed == _SYSV_B134 ? 134 :
     termio_speed == _SYSV_B150 ? 150 :
     termio_speed == _SYSV_B200 ? 200 :
     termio_speed == _SYSV_B300 ? 300 :
     termio_speed == _SYSV_B600 ? 600 :
     termio_speed == _SYSV_B1200 ? 1200 :
     termio_speed == _SYSV_B1800 ? 1800 :
     termio_speed == _SYSV_B2400 ? 2400 :
     termio_speed == _SYSV_B4800 ? 4800 :
     termio_speed == _SYSV_B9600 ? 9600 :
     termio_speed == _SYSV_B19200 ? 19200 :
     termio_speed == _SYSV_B38400 ? 38400 :
     -1);
  termios_p->__ispeed = termios_p->__ospeed;

  termios_p->c_iflag = 0;
  if (buf.c_iflag & _SYSV_IGNBRK)
    termios_p->c_iflag |= IGNBRK;
  if (buf.c_iflag & _SYSV_BRKINT)
    termios_p->c_iflag |= BRKINT;
  if (buf.c_iflag & _SYSV_IGNPAR)
    termios_p->c_iflag |= IGNPAR;
  if (buf.c_iflag & _SYSV_PARMRK)
    termios_p->c_iflag |= PARMRK;
  if (buf.c_iflag & _SYSV_INPCK)
    termios_p->c_iflag |= INPCK;
  if (buf.c_iflag & _SYSV_ISTRIP)
    termios_p->c_iflag |= ISTRIP;
  if (buf.c_iflag & _SYSV_INLCR)
    termios_p->c_iflag |= INLCR;
  if (buf.c_iflag & _SYSV_IGNCR)
    termios_p->c_iflag |= IGNCR;
  if (buf.c_iflag & _SYSV_ICRNL)
    termios_p->c_iflag |= ICRNL;
  if (buf.c_iflag & _SYSV_IXON)
    termios_p->c_iflag |= IXON;
  if (buf.c_iflag & _SYSV_IXOFF)
    termios_p->c_iflag |= IXOFF;
  if (buf.c_iflag & _SYSV_IXANY)
    termios_p->c_iflag |= IXANY;
  if (buf.c_iflag & _SYSV_IMAXBEL)
    termios_p->c_iflag |= IMAXBEL;

  termios_p->c_oflag = 0;
  if (buf.c_oflag & OPOST)
    termios_p->c_oflag |= OPOST;
  if (buf.c_oflag & ONLCR)
    termios_p->c_oflag |= ONLCR;
  termios_p->c_cflag = 0;
  switch (buf.c_cflag & _SYSV_CSIZE)
    {
    case _SYSV_CS5:
      termios_p->c_cflag |= CS5;
      break;
    case _SYSV_CS6:
      termios_p->c_cflag |= CS6;
      break;
    case _SYSV_CS7:
      termios_p->c_cflag |= CS7;
      break;
    case _SYSV_CS8:
      termios_p->c_cflag |= CS8;
      break;
    }
  if (buf.c_cflag & _SYSV_CSTOPB)
    termios_p->c_cflag |= CSTOPB;
  if (buf.c_cflag & _SYSV_CREAD)
    termios_p->c_cflag |= CREAD;
  if (buf.c_cflag & _SYSV_PARENB)
    termios_p->c_cflag |= PARENB;
  if (buf.c_cflag & _SYSV_PARODD)
    termios_p->c_cflag |= PARODD;
  if (buf.c_cflag & _SYSV_HUPCL)
    termios_p->c_cflag |= HUPCL;
  if (buf.c_cflag & _SYSV_CLOCAL)
    termios_p->c_cflag |= CLOCAL;
  termios_p->c_lflag = 0;
  if (buf.c_lflag & _SYSV_ISIG)
    termios_p->c_lflag |= _ISIG;
  if (buf.c_lflag & _SYSV_ICANON)
    termios_p->c_lflag |= _ICANON;
  if (buf.c_lflag & _SYSV_ECHO)
    termios_p->c_lflag |= _ECHO;
  if (buf.c_lflag & _SYSV_ECHOE)
    termios_p->c_lflag |= _ECHOE;
  if (buf.c_lflag & _SYSV_ECHOK)
    termios_p->c_lflag |= _ECHOK;
  if (buf.c_lflag & _SYSV_ECHONL)
    termios_p->c_lflag |= _ECHONL;
  if (buf.c_lflag & _SYSV_NOFLSH)
    termios_p->c_lflag |= _NOFLSH;
  if (buf.c_lflag & _SYSV_TOSTOP)
    termios_p->c_lflag |= _TOSTOP;
  if (buf.c_lflag & _SYSV_ECHOKE)
    termios_p->c_lflag |= ECHOKE;
  if (buf.c_lflag & _SYSV_ECHOPRT)
    termios_p->c_lflag |= ECHOPRT;
  if (buf.c_lflag & _SYSV_ECHOCTL)
    termios_p->c_lflag |= ECHOCTL;
  if (buf.c_lflag & _SYSV_FLUSHO)
    termios_p->c_lflag |= FLUSHO;
  if (buf.c_lflag & _SYSV_PENDIN)
    termios_p->c_lflag |= PENDIN;
  if (buf.c_lflag & _SYSV_IEXTEN)
    termios_p->c_lflag |= IEXTEN;

  termios_p->c_cc[VEOF] = buf.c_cc[_SYSV_VEOF];
  termios_p->c_cc[VEOL] = buf.c_cc[_SYSV_VEOL];
  termios_p->c_cc[VEOL2] = buf.c_cc[_SYSV_VEOL2];
  termios_p->c_cc[VERASE] = buf.c_cc[_SYSV_VERASE];
  termios_p->c_cc[VKILL] = buf.c_cc[_SYSV_VKILL];
  termios_p->c_cc[VINTR] = buf.c_cc[_SYSV_VINTR];
  termios_p->c_cc[VQUIT] = buf.c_cc[_SYSV_VQUIT];
  termios_p->c_cc[VSTART] = '\021'; /* XON (^Q).  */
  termios_p->c_cc[VSTOP] = '\023'; /* XOFF (^S).  */
  termios_p->c_cc[VSUSP] = '\0'; /* System V release 3 lacks job control.  */
  termios_p->c_cc[VMIN] = buf.c_cc[_SYSV_VMIN];
  termios_p->c_cc[VTIME] = buf.c_cc[_SYSV_VTIME];

  return 0;
}

weak_alias (__tcgetattr, tcgetattr)
