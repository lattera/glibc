/* Copyright (C) 1992, 1995, 1996, 1997, 2002 Free Software Foundation, Inc.
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
#include <termios.h>
#include <sys/ioctl.h>

#include <sysv_termio.h>


const speed_t __unix_speeds[] =
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


/* Set the state of FD to *TERMIOS_P.  */
int
tcsetattr (fd, optional_actions, termios_p)
     int fd;
     int optional_actions;
     const struct termios *termios_p;
{
  struct __sysv_termio buf;
  int ioctl_function;
  size_t i;

  if (termios_p == NULL)
    {
      __set_errno (EINVAL);
      return -1;
    }
  switch (optional_actions)
    {
    case TCSANOW:
      ioctl_function = _TCSETA;
      break;
    case TCSADRAIN:
      ioctl_function = _TCSETAW;
      break;
    case TCSAFLUSH:
      ioctl_function = _TCSETAF;
      break;
    default:
      __set_errno (EINVAL);
      return -1;
    }

  if (termios_p->__ispeed != termios_p->__ospeed)
    {
      __set_errno (EINVAL);
      return -1;
    }
  buf.c_cflag = -1;
  for (i = 0; i <= sizeof (__unix_speeds) / sizeof (__unix_speeds[0]); ++i)
    {
      if (__unix_speeds[i] == termios_p->__ispeed)
	buf.c_cflag = i;
    }
  if (buf.c_cflag == -1)
    {
      __set_errno (EINVAL);
      return -1;
    }

  buf.c_iflag = 0;
  if (termios_p->c_iflag & IGNBRK)
    buf.c_iflag |= _SYSV_IGNBRK;
  if (termios_p->c_iflag & BRKINT)
    buf.c_iflag |= _SYSV_BRKINT;
  if (termios_p->c_iflag & IGNPAR)
    buf.c_iflag |= _SYSV_IGNPAR;
  if (termios_p->c_iflag & PARMRK)
    buf.c_iflag |= _SYSV_PARMRK;
  if (termios_p->c_iflag & INPCK)
    buf.c_iflag |= _SYSV_INPCK;
  if (termios_p->c_iflag & ISTRIP)
    buf.c_iflag |= _SYSV_ISTRIP;
  if (termios_p->c_iflag & INLCR)
    buf.c_iflag |= _SYSV_INLCR;
  if (termios_p->c_iflag & IGNCR)
    buf.c_iflag |= _SYSV_IGNCR;
  if (termios_p->c_iflag & ICRNL)
    buf.c_iflag |= _SYSV_ICRNL;
  if (termios_p->c_iflag & IXON)
    buf.c_iflag |= _SYSV_IXON;
  if (termios_p->c_iflag & IXOFF)
    buf.c_iflag |= _SYSV_IXOFF;
  if (termios_p->c_iflag & IXANY)
    buf.c_iflag |= _SYSV_IXANY;
  if (termios_p->c_iflag & IMAXBEL)
    buf.c_iflag |= _SYSV_IMAXBEL;

  buf.c_oflag = 0;
  if (termios_p->c_oflag & OPOST)
    buf.c_oflag |= _SYSV_OPOST;
  if (termios_p->c_oflag & ONLCR)
    buf.c_oflag |= _SYSV_ONLCR;

  /* So far, buf.c_cflag contains the speed in CBAUD.  */
  if (termios_p->c_cflag & CSTOPB)
    buf.c_cflag |= _SYSV_CSTOPB;
  if (termios_p->c_cflag & CREAD)
    buf.c_cflag |= _SYSV_CREAD;
  if (termios_p->c_cflag & PARENB)
    buf.c_cflag |= _SYSV_PARENB;
  if (termios_p->c_cflag & PARODD)
    buf.c_cflag |= _SYSV_PARODD;
  if (termios_p->c_cflag & HUPCL)
    buf.c_cflag |= _SYSV_HUPCL;
  if (termios_p->c_cflag & CLOCAL)
    buf.c_cflag |= _SYSV_CLOCAL;
  switch (termios_p->c_cflag & CSIZE)
    {
    case CS5:
      buf.c_cflag |= _SYSV_CS5;
      break;
    case CS6:
      buf.c_cflag |= _SYSV_CS6;
      break;
    case CS7:
      buf.c_cflag |= _SYSV_CS7;
      break;
    case CS8:
      buf.c_cflag |= _SYSV_CS8;
      break;
    }

  buf.c_lflag = 0;
  if (termios_p->c_lflag & ISIG)
    buf.c_lflag |= _SYSV_ISIG;
  if (termios_p->c_lflag & ICANON)
    buf.c_lflag |= _SYSV_ICANON;
  if (termios_p->c_lflag & ECHO)
    buf.c_lflag |= _SYSV_ECHO;
  if (termios_p->c_lflag & ECHOE)
    buf.c_lflag |= _SYSV_ECHOE;
  if (termios_p->c_lflag & ECHOK)
    buf.c_lflag |= _SYSV_ECHOK;
  if (termios_p->c_lflag & ECHONL)
    buf.c_lflag |= _SYSV_ECHONL;
  if (termios_p->c_lflag & NOFLSH)
    buf.c_lflag |= _SYSV_NOFLSH;
  if (termios_p->c_lflag & TOSTOP)
    buf.c_lflag |= _SYSV_TOSTOP;
  if (termios_p->c_lflag & ECHOCTL)
    buf.c_lflag |= _SYSV_ECHOCTL;
  if (termios_p->c_lflag & ECHOPRT)
    buf.c_lflag |= _SYSV_ECHOPRT;
  if (termios_p->c_lflag & ECHOKE)
    buf.c_lflag |= _SYSV_ECHOKE;
  if (termios_p->c_lflag & FLUSHO)
    buf.c_lflag |= _SYSV_FLUSHO;
  if (termios_p->c_lflag & PENDIN)
    buf.c_lflag |= _SYSV_PENDIN;
  if (termios_p->c_lflag & IEXTEN)
    buf.c_lflag |= _SYSV_IEXTEN;

  buf.c_cc[_SYSV_VINTR] = termios_p->c_cc[VINTR];
  buf.c_cc[_SYSV_VQUIT] = termios_p->c_cc[VQUIT];
  buf.c_cc[_SYSV_VERASE] = termios_p->c_cc[VERASE];
  buf.c_cc[_SYSV_VKILL] = termios_p->c_cc[VKILL];
  if (buf.c_lflag & _SYSV_ICANON)
    {
      buf.c_cc[_SYSV_VEOF] = termios_p->c_cc[VEOF];
      buf.c_cc[_SYSV_VEOL] = termios_p->c_cc[VEOL];
    }
  else
    {
      buf.c_cc[_SYSV_VMIN] = termios_p->c_cc[VMIN];
      buf.c_cc[_SYSV_VTIME] = termios_p->c_cc[VTIME];
    }
  buf.c_cc[_SYSV_VEOL2] = termios_p->c_cc[VEOL2];

  if (__ioctl (fd, ioctl_function, &buf) < 0)
    return -1;
  return 0;
}
libc_hidden_def (tcsetattr)
