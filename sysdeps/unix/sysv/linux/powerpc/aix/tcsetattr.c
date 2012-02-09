/* Copyright (C) 2000 Free Software Foundation, Inc.
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
#include <termios.h>
#include "aix-termios.h"

int
tcsetattr (fd, optional_actions, linuxtermios_p)
     int fd;
     int optional_actions;
     const struct termios *linuxtermios_p;
{
  struct aixtermios aixtermios;
  int result;

  /* `optional_actions' does not have to be changed, AIX uses the
     same values as Linux.  */

  aixtermios.c_cc[AIX_VINTR] = linuxtermios_p->c_cc[VINTR];
  aixtermios.c_cc[AIX_VQUIT] = linuxtermios_p->c_cc[VQUIT];
  aixtermios.c_cc[AIX_VERASE] = linuxtermios_p->c_cc[VERASE];
  aixtermios.c_cc[AIX_VKILL] = linuxtermios_p->c_cc[VKILL];
  aixtermios.c_cc[AIX_VEOF] = linuxtermios_p->c_cc[VEOF];
  // XXX VMIN has the same value as VEOF !?
  aixtermios.c_cc[AIX_VEOL] = linuxtermios_p->c_cc[VEOL];
  // XXX VTIME has the same value as VEOL !?
  aixtermios.c_cc[AIX_VEOL2] = linuxtermios_p->c_cc[VEOL2];
  aixtermios.c_cc[AIX_VSTART] = linuxtermios_p->c_cc[VSTART];
  aixtermios.c_cc[AIX_VSTOP] = linuxtermios_p->c_cc[VSTOP];
  aixtermios.c_cc[AIX_VSUSP] = linuxtermios_p->c_cc[VSUSP];
  aixtermios.c_cc[AIX_VDSUSP] = 0; // XXX No Linux equivalent !?
  aixtermios.c_cc[AIX_VREPRINT] = linuxtermios_p->c_cc[VREPRINT];
  aixtermios.c_cc[AIX_VDISCARD] = linuxtermios_p->c_cc[VDISCARD];
  aixtermios.c_cc[AIX_VWERASE] = linuxtermios_p->c_cc[VWERASE];
  aixtermios.c_cc[AIX_VLNEXT] = linuxtermios_p->c_cc[VLNEXT];

  /* AIX has not all the speeds (the high one) Linux supports.  The
     symbol names and values used for the speeds are fortunately the
     same.  */
  if ((linuxtermios_p->c_cflag & CBAUD) > B38400)
    {
      __set_errno (EINVAL);
      return -1;
    }

  aixtermios.c_c_flag = linuxtermios_p->c_cflag & CBAUD;

  /* Only the IUCLC, IXANY, and IMAXBEL values are different in the
     c_iflag member.  */
  aixtermios.c_iflag = linuxtermios_p->c_iflag & 0x7ff;
  if (linuxtermios_p->c_iflag & IXANY)
    aixtermios.c_iflag |= AIX_IXANY;
  if (linuxtermios_p->c_iflag & IUCLC)
    aixtermios.c_iflag |= AIX_IUCLC;
  if (linuxtermios_p->c_iflag & IMAXBEL)
    aixtermios.c_iflag |= AIX_IMAXBEL;

  /* Many of the c_oflag files differ.  Bummer.  */
  aixtermios.c_oflag = (linuxtermios_p->c_oflag
			& (OPOST | OCRNL | ONOCR | ONLRET | OFILL
			   | OFDEL | TABDLY));
  if (linuxtermios_p->c_oflag & OLCUC)
    aixtermios.c_oflag |= AIX_OLCUC;
  if (linuxtermios_p->c_oflag & ONLCR)
    aixtermios.c_oflag |= AIX_ONLCR;
  if (linuxtermios_p->c_oflag & NLDLY)
    {
      if ((linuxtermios_p->c_oflag & NLDLY) >= NL2)
	{
	  __set_errno (EINVAL);
	  return -1;
	}

      if (linuxtermios_p->c_oflag & NLDLY)
	aixtermios.c_oflag |= AIX_NL1;
    }
  if (linuxtermios_p->c_oflag & TABDLY)
    {
#define offset 2
#if TAB1 >> offset != AIX_TAB1 || TAB3 >> offset != AIX_TAB3
# error "Check the offset"
#endif
      aixtermios.c_oflag |= (linuxtermios_p->c_oflag >> offset) & AIX_TABDLY;
#undef offset
    }
  if (linuxtermios_p->c_oflag & FFDLY)
    aixtermios.c_oflag |= AIX_FF1;
  if (linuxtermios_p->c_oflag & BSDLY)
    aixtermios.c_oflag |= AIX_BS1;
  if (linuxtermios_p->c_oflag & VTDLY)
    aixtermios.c_oflag |= AIX_VT1;

  /* A lot of the c_cflag member is also different.  */
  if (linuxtermios_p->c_cflag & CSIZE)
    {
#define offset 4
#if CSIZE >> offset != AIX_CSIZE
# error "Check the offset"
#endif
      aixtermios.c_cflag |= (linuxtermios_p->c_cflag >> offset) & AIX_CSIZE;
#undef offset
    }

  if (linuxtermios_p->c_cflag & STOPB)
    aixtermios.c_cflag |= AIX_STOPB;
  if (linuxtermios_p->c_cflag & CREAD)
    aixtermios.c_cflag |= AIX_CREAD;
  if (linuxtermios_p->c_cflag & PARENB)
    aixtermios.c_cflag |= AIX_PARENB;
  if (linuxtermios_p->c_cflag & PARODD)
    aixtermios.c_cflag |= AIX_PARODD;
  if (linuxtermios_p->c_cflag & HUPCL)
    aixtermios.c_cflag |= AIX_HUPCL;
  if (linuxtermios_p->c_cflag & CLOCAL)
    aixtermios.c_cflag |= AIX_CLOCAL;

  /* The c_lflag is information is also different.  */
  aixtermios.c_lflag = 0;
  if (linuxtermios_p->c_lflag & ISIG)
    aixtermios.c_lflag |= AIX_ISIG;
  if (linuxtermios_p->c_lflag & ICANON)
    aixtermios.c_lflag |= AIX_ICANON;
  if (linuxtermios_p->c_lflag & XCASE)
    aixtermios.c_lflag |= AIX_XCASE;
  if (linuxtermios_p->c_lflag & ECHO)
    aixtermios.c_lflag |= AIX_ECHO;
  if (linuxtermios_p->c_lflag & ECHOE)
    aixtermios.c_lflag |= AIX_ECHOE;
  if (linuxtermios_p->c_lflag & ECHOK)
    aixtermios.c_lflag |= AIX_ECHOK;
  if (linuxtermios_p->c_lflag & ECHONL)
    aixtermios.c_lflag |= AIX_ECHONL;
  if (linuxtermios_p->c_lflag & NOFLSH)
    aixtermios.c_lflag |= AIX_NOFLSH;
  if (linuxtermios_p->c_lflag & TOSTOP)
    aixtermios.c_lflag |= AIX_TOSTOP;
  if (linuxtermios_p->c_lflag & ECHOCTL)
    aixtermios.c_lflag |= AIX_ECHOCTL;
  if (linuxtermios_p->c_lflag & ECHOPRT)
    aixtermios.c_lflag |= AIX_ECHOPRT;
  if (linuxtermios_p->c_lflag & ECHOKE)
    aixtermios.c_lflag |= AIX_ECHOKE;
  if (linuxtermios_p->c_lflag & FLUSHO)
    aixtermios.c_lflag |= AIX_FLUSHO;
  if (linuxtermios_p->c_lflag & PENDIN)
    aixtermios.c_lflag |= AIX_PENDIN;
  if (linuxtermios_p->c_lflag & IEXTEN)
    aixtermios.c_lflag |= AIX_IEXTEN;

  result = /* XXX syscall */;

  // Convert error here or in syscall.

  return result;
}
