/* Structure types for pre-termios terminal ioctls.  AIX version.
   Copyright (C) 1999, 2000, 2002 Free Software Foundation, Inc.
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

#ifndef _SYS_IOCTL_H
# error "Never use <bits/ioctl-types.h> directly; include <sys/ioctl.h> instead."
#endif

/* Constants for use with `ioctl'.  */
#define TIOC		('T' << 8)
#define TCGETS		(TIOC | 1)
#define TCSETS		(TIOC | 2)
#define TCSETSW		(TIOC | 3)
#define TCSETSF		(TIOC | 4)
#define TCGETA		(TIOC | 5)
#define TCSETA		(TIOC | 6)
#define TCSETAW		(TIOC | 7)
#define TCSETAF		(TIOC | 8)
#define TCSBRK		(TIOC | 9)
#define TCSBREAK	(TIOC | 10)
#define TCXONC		(TIOC | 11)
#define TCFLSH		(TIOC | 12)
#define TCGLEN		(TIOC | 13)
#define TCSLEN		(TIOC | 14)
#define TCSAK		(TIOC | 15)
#define TCQSAK		(TIOC | 16)
#define TCTRUST		(TIOC | 17)
#define TCQTRUST	(TIOC | 18)
#define TCSMAP		(TIOC | 19)
#define TCGMAP		(TIOC | 20)
#define TCKEP		(TIOC | 21)
#define TCGSAK		(TIOC | 22)
#define TCLOOP		(TIOC | 23)
#define TCVPD		(TIOC | 24)
#define TCREG		(TIOC | 25)
#define TCGSTATUS	(TIOC | 26)
#define TCSCONTROL	(TIOC | 27)
#define TCSCSMAP	(TIOC | 28)
#define TCGCSMAP	(TIOC | 29)
#define TCMGR		TCSAK
#define TCQMGR		TCQSAK
#define TIONREAD	FIONREAD



struct winsize
{
  unsigned short int ws_row;
  unsigned short int ws_col;
  unsigned short int ws_xpixel;
  unsigned short int ws_ypixel;
};

#define NCC 8
struct termio
{
  unsigned short int c_iflag;		/* input mode flags */
  unsigned short int c_oflag;		/* output mode flags */
  unsigned short int c_cflag;		/* control mode flags */
  unsigned short int c_lflag;		/* local mode flags */
  char c_line;				/* line discipline */
  unsigned char c_cc[NCC];		/* control characters */
};

/* modem lines */
#define TIOCM_LE	0x001
#define TIOCM_DTR	0x002
#define TIOCM_RTS	0x004
#define TIOCM_ST	0x008
#define TIOCM_SR	0x010
#define TIOCM_CTS	0x020
#define TIOCM_CAR	0x040
#define TIOCM_RNG	0x080
#define TIOCM_DSR	0x100
#define TIOCM_CD	TIOCM_CAR
#define TIOCM_RI	TIOCM_RNG
