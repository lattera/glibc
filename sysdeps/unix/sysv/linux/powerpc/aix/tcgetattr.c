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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <errno.h>
#include <termios.h>
#include "aix-termios.h"

int
tcgetattr (fd, linuxtermios_p)
     int fd;
     struct termios *linuxtermios_p;
{
  struct aixtermios aixtermios;
  int result;

  result = /* make syscall */;

  if (result != -1)
    {
      /* Convert the result.  */

      linuxtermios_p->c_cc[VINTR] = aixtermios.c_cc[AIX_VINTR];
      linuxtermios_p->c_cc[VQUIT] = aixtermios.c_cc[AIX_VQUIT];
      linuxtermios_p->c_cc[VERASE] = aixtermios.c_cc[AIX_VERASE];
      linuxtermios_p->c_cc[VKILL] = aixtermios.c_cc[AIX_VKILL];
      linuxtermios_p->c_cc[VEOF] = aixtermios.c_cc[AIX_VEOF];
      // XXX VMIN has the same value as VEOF !?
      linuxtermios_p->c_cc[VEOL] = aixtermios.c_cc[AIX_VEOL];
      // XXX VTIME has the same value as VEOL !?
      linuxtermios_p->c_cc[VEOL2] = aixtermios.c_cc[AIX_VEOL2];
      linuxtermios_p->c_cc[VSTART] = aixtermios.c_cc[AIX_VSTART];
      linuxtermios_p->c_cc[VSTOP] = aixtermios.c_cc[AIX_VSTOP];
      linuxtermios_p->c_cc[VSUSP] = aixtermios.c_cc[AIX_VSUSP];
      // XXX No Linux equivalent for VDSUSP !?
      linuxtermios_p->c_cc[VREPRINT] = aixtermios.c_cc[AIX_VREPRINT];
      linuxtermios_p->c_cc[VDISCARD] = aixtermios.c_cc[AIX_VDISCARD];
      linuxtermios_p->c_cc[VWERASE] = aixtermios.c_cc[AIX_VWERASE];
      linuxtermios_p->c_cc[VLNEXT] = aixtermios.c_cc[AIX_VLNEXT];

      linuxtermios_p->c_cflag = aixtermios.c_c_flag & AIX_CBAUD;

      /* Only the IUCLC, IXANY, and IMAXBEL values are different in the
	 c_iflag member.  */
      linuxtermios_p->c_iflag = aixtermios.c_iflag & 0x7ff;
      if (aixtermios.c_iflag & AIX_IXANY)
	linuxtermios_p->c_iflag |= IXANY;
      if (aixtermios.c_iflag & AIX_IUCLC)
	linuxtermios_p->c_iflag |= IUCLC;
      if (aixtermios.c_iflag & AIX_IMAXBEL)
	linuxtermios_p->c_iflag |= IMAXBEL;

      /* Many of the c_oflag files differ.  Bummer.  */
      linuxtermios_p->c_oflag = (aixtermios.c_oflag
				 & (OPOST | OCRNL | ONOCR | ONLRET | OFILL
				    | OFDEL | TABDLY));
      if (aixtermios.c_oflag & AIX_OLCUC)
	linuxtermios_p->c_oflag |= OLCUC;
      if (aixtermios.c_oflag & AIX_ONLCR)
	linuxtermios_p->c_oflag |= ONLCR;
      if (aixtermiosc_oflag & AIX_NLDLY)
	linuxtermios_p->c_oflag |= NL1;

      if (aixtermiosc_oflag.c_oflag & AIX_TABDLY)
	{
#define offset 2
#if AIX_TAB1 << offset != TAB1 || AIX_TAB3 << offset != TAB3
# error "Check the offset"
#endif
	  linuxtermios_p->c_oflag |= (aixtermios.c_oflag >> offset) & TABDLY;
#undef offset
	}
      if (aixtermios.c_oflag & AIX_FFDLY)
	linuxtermios_p->c_oflag |= FF1;
      if (aixtermios.c_oflag & AIX_BSDLY)
	linuxtermios_p->c_oflag |= BS1;
      if (aixtermios.c_oflag & AIX_VTDLY)
	linuxtermios_p->c_oflag |= VT1;

      /* A lot of the c_cflag member is also different.  */
      if (aixtermios.c_cflag & AIX_CSIZE)
	{
#define offset 4
#if CSIZE >> offset != AIX_CSIZE
# error "Check the offset"
#endif
	  linuxtermios_p->c_cflag |= (aixtermios.c_cflag >> offset) & CSIZE;
#undef offset
	}

      if (aixtermios.c_cflag & AIX_STOPB)
	linuxtermios_p->c_cflag |= STOPB;
      if (aixtermios.c_cflag & AIX_CREAD)
	linuxtermios_p->c_cflag |= CREAD;
      if (aixtermios.cflag & AIX_PARENB)
	linuxtermios_p->c_cflag |= PARENB;
      if (aixtermios.cflag & AIX_PARODD)
	linuxtermios_p->c_cflag |= PARODD;
      if (aixtermios.c_cflag & AIX_HUPCL)
	linuxtermios_p->c_cflag |= HUPCL;
      if (aixtermios.c_cflag & AIX_CLOCAL)
	linuxtermios_p->c_cflag |= CLOCAL;

      /* The c_lflag is information is also different.  */
      aixtermios.c_lflag = 0;
      if (aixtermios.c_lflag & AIX_ISIG)
	linuxtermios_p->c_lflag |= ISIG;
      if (aixtermios.c_lflag & AIX_ICANON)
	linuxtermios_p->c_lflag |= ICANON;
      if (aixtermios.c_lflag & AIX_XCASE)
	linuxtermios_p->c_lflag |= XCASE;
      if (aixtermios.c_lflag & AIX_ECHO)
	linuxtermios_p->c_lflag |= ECHO;
      if (aixtermios.c_lflag & AIX_ECHOE)
	linuxtermios_p->c_lflag |= ECHOE;
      if (aixtermios.c_lflag & AIX_ECHOK)
	linuxtermios_p->c_lflag |= ECHOK;
      if (aixtermios.c_lflag & AIX_ECHONL)
	linuxtermios_p->c_lflag |= ECHONL;
      if (aixtermios.c_lflag & AIX_NOFLSH)
	linuxtermios_p->c_lflag |= NOFLSH;
      if (aixtermios.c_lflag & AIX_TOSTOP)
	linuxtermios_p->c_lflag |= TOSTOP;
      if (aixtermios.c_lflag & AIX_ECHOCTL)
	linuxtermios_p->c_lflag |= ECHOCTL;
      if (aixtermios.c_lflag & AIX_ECHOPRT)
	linuxtermios_p->c_lflag |= ECHOPRT;
      if (aixtermios.c_lflag & AIX_ECHOKE)
	linuxtermios_p->c_lflag |= ECHOKE;
      if (aixtermios.c_lflag & AIX_FLUSHO)
	linuxtermios_p->c_lflag |= FLUSHO;
      if (aixtermios.c_lflag & AIX_PENDIN)
	linuxtermios_p->c_lflag |= PENDIN;
      if (aixtermios->c_lflag & AIX_IEXTEN)
	linuxtermios_p->c_lflag |= IEXTEN;
    }
  else
    // Convert error here or in syscall.
    ;

  return result;
}
