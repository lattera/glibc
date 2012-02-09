/* Copyright (C) 1991, 1992, 1995, 1996, 1997 Free Software Foundation, Inc.
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

#include "bsdtty.h"

extern const speed_t __bsd_speeds[]; /* Defined in tcsetattr.c.  */

/* Put the state of FD into *TERMIOS_P.  */
int
__tcgetattr (fd, termios_p)
     int fd;
     struct termios *termios_p;
{
  struct sgttyb buf;
  struct tchars tchars;
  struct ltchars ltchars;
  int local;
#ifdef	TIOCGETX
  int extra;
#endif

  if (termios_p == NULL)
    {
      __set_errno (EINVAL);
      return -1;
    }

  if (__ioctl(fd, TIOCGETP, &buf) < 0 ||
      __ioctl(fd, TIOCGETC, &tchars) < 0 ||
      __ioctl(fd, TIOCGLTC, &ltchars) < 0 ||
#ifdef	TIOCGETX
      __ioctl(fd, TIOCGETX, &extra) < 0 ||
#endif
      __ioctl(fd, TIOCLGET, &local) < 0)
    return -1;

  termios_p->__ispeed = __bsd_speeds[(unsigned char) buf.sg_ispeed];
  termios_p->__ospeed = __bsd_speeds[(unsigned char) buf.sg_ospeed];

  termios_p->c_iflag = 0;
  termios_p->c_oflag = 0;
  termios_p->c_cflag = 0;
  termios_p->c_lflag = 0;
  termios_p->c_oflag |= CREAD | HUPCL;
#ifdef	LPASS8
  if (local & LPASS8)
    termios_p->c_oflag |= CS8;
  else
#endif
    termios_p->c_oflag |= CS7;
  if (!(buf.sg_flags & RAW))
    {
      termios_p->c_iflag |= IXON;
      termios_p->c_cflag |= OPOST;
#ifndef	NOISIG
      termios_p->c_lflag |= ISIG;
#endif
    }
  if ((buf.sg_flags & (CBREAK|RAW)) == 0)
    termios_p->c_lflag |= ICANON;
  if (!(buf.sg_flags & RAW) && !(local & LLITOUT))
    termios_p->c_oflag |= OPOST;
  if (buf.sg_flags & CRMOD)
    termios_p->c_iflag |= ICRNL;
  if (buf.sg_flags & TANDEM)
    termios_p->c_iflag |= IXOFF;
#ifdef	TIOCGETX
  if (!(extra & NOISIG))
    termios_p->c_lflag |= ISIG;
  if (extra & STOPB)
    termios_p->c_cflag |= CSTOPB;
#endif

  switch (buf.sg_flags & (EVENP|ODDP))
    {
    case EVENP|ODDP:
      break;
    case ODDP:
      termios_p->c_cflag |= PARODD;
    default:
      termios_p->c_cflag |= PARENB;
      termios_p->c_iflag |= IGNPAR | INPCK;
      break;
    }
  if (buf.sg_flags & ECHO)
    termios_p->c_lflag |= _ECHO;
  if (local & LCRTERA)
    termios_p->c_lflag |= ECHOE;
  if (local & LCRTKIL)
    termios_p->c_lflag |= ECHOK;
  if (local & LTOSTOP)
    termios_p->c_lflag |= _TOSTOP;
  if (local & LNOFLSH)
    termios_p->c_lflag |= _NOFLSH;

  termios_p->c_cc[VEOF] = tchars.t_eofc;
  termios_p->c_cc[VEOL] = '\n';
  termios_p->c_cc[VERASE] = buf.sg_erase;
  termios_p->c_cc[VKILL] = buf.sg_kill;
  termios_p->c_cc[VINTR] = tchars.t_intrc;
  termios_p->c_cc[VQUIT] = tchars.t_quitc;
  termios_p->c_cc[VSTART] = tchars.t_startc;
  termios_p->c_cc[VSTOP] = tchars.t_stopc;
  termios_p->c_cc[VSUSP] = ltchars.t_suspc;
  termios_p->c_cc[VMIN] = -1;
  termios_p->c_cc[VTIME] = -1;

  return 0;
}

weak_alias (__tcgetattr, tcgetattr)
