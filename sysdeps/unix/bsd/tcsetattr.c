/* Copyright (C) 1991, 1993, 1996, 1997, 2002 Free Software Foundation, Inc.
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

#include <errno.h>
#include <stddef.h>
#include <termios.h>

#include "bsdtty.h"


const speed_t __bsd_speeds[] =
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
  struct sgttyb buf;
  struct tchars tchars;
  struct ltchars ltchars;
  int local;
#ifdef	TIOCGETX
  int extra;
#endif
  size_t i;

  if (__ioctl (fd, TIOCGETP, &buf) < 0 ||
      __ioctl (fd, TIOCGETC, &tchars) < 0 ||
      __ioctl (fd, TIOCGLTC, &ltchars) < 0 ||
#ifdef	TIOCGETX
      __ioctl (fd, TIOCGETX, &extra) < 0 ||
#endif
      __ioctl (fd, TIOCLGET, &local) < 0)
    return -1;

  if (termios_p == NULL)
    {
      __set_errno (EINVAL);
      return -1;
    }
  switch (optional_actions)
    {
    case TCSANOW:
      break;
    case TCSADRAIN:
      if (tcdrain (fd) < 0)
	return -1;
      break;
    case TCSAFLUSH:
      if (tcflush (fd, TCIFLUSH) < 0)
	return -1;
      break;
    default:
      __set_errno (EINVAL);
      return -1;
    }

  buf.sg_ispeed = buf.sg_ospeed = -1;
  for (i = 0; i <= sizeof (__bsd_speeds) / sizeof (__bsd_speeds[0]); ++i)
    {
      if (__bsd_speeds[i] == termios_p->__ispeed)
	buf.sg_ispeed = i;
      if (__bsd_speeds[i] == termios_p->__ospeed)
	buf.sg_ospeed = i;
    }
  if (buf.sg_ispeed == -1 || buf.sg_ospeed == -1)
    {
      __set_errno (EINVAL);
      return -1;
    }

  buf.sg_flags &= ~(CBREAK|RAW);
  if (!(termios_p->c_lflag & ICANON))
    buf.sg_flags |= (termios_p->c_cflag & ISIG) ? CBREAK : RAW;
#ifdef	LPASS8
  if (termios_p->c_oflag & CS8)
    local |= LPASS8;
  else
    local &= ~LPASS8;
#endif
  if (termios_p->c_lflag & _NOFLSH)
    local |= LNOFLSH;
  else
    local &= ~LNOFLSH;
  if (termios_p->c_oflag & OPOST)
    local &= ~LLITOUT;
  else
    local |= LLITOUT;
#ifdef	TIOCGETX
  if (termios_p->c_lflag & ISIG)
    extra &= ~NOISIG;
  else
    extra |= NOISIG;
  if (termios_p->c_cflag & CSTOPB)
    extra |= STOPB;
  else
    extra &= ~STOPB;
#endif
  if (termios_p->c_iflag & ICRNL)
    buf.sg_flags |= CRMOD;
  else
    buf.sg_flags &= ~CRMOD;
  if (termios_p->c_iflag & IXOFF)
    buf.sg_flags |= TANDEM;
  else
    buf.sg_flags &= ~TANDEM;

  buf.sg_flags &= ~(ODDP|EVENP);
  if (!(termios_p->c_cflag & PARENB))
    buf.sg_flags |= ODDP | EVENP;
  else if (termios_p->c_cflag & PARODD)
    buf.sg_flags |= ODDP;
  else
    buf.sg_flags |= EVENP;

  if (termios_p->c_lflag & _ECHO)
    buf.sg_flags |= ECHO;
  else
    buf.sg_flags &= ~ECHO;
  if (termios_p->c_lflag & ECHOE)
    local |= LCRTERA;
  else
    local &= ~LCRTERA;
  if (termios_p->c_lflag & ECHOK)
    local |= LCRTKIL;
  else
    local &= ~LCRTKIL;
  if (termios_p->c_lflag & _TOSTOP)
    local |= LTOSTOP;
  else
    local &= ~LTOSTOP;

  buf.sg_erase = termios_p->c_cc[VERASE];
  buf.sg_kill = termios_p->c_cc[VKILL];
  tchars.t_eofc = termios_p->c_cc[VEOF];
  tchars.t_intrc = termios_p->c_cc[VINTR];
  tchars.t_quitc = termios_p->c_cc[VQUIT];
  ltchars.t_suspc = termios_p->c_cc[VSUSP];
  tchars.t_startc = termios_p->c_cc[VSTART];
  tchars.t_stopc = termios_p->c_cc[VSTOP];

  if (__ioctl (fd, TIOCSETP, &buf) < 0 ||
      __ioctl (fd, TIOCSETC, &tchars) < 0 ||
      __ioctl (fd, TIOCSLTC, &ltchars) < 0 ||
#ifdef	TIOCGETX
      __ioctl (fd, TIOCSETX, &extra) < 0 ||
#endif
      __ioctl (fd, TIOCLSET, &local) < 0)
    return -1;
  return 0;
}
libc_hidden_def (tcsetattr)
