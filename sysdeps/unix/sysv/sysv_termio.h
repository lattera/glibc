/* Copyright (C) 1992, 1997 Free Software Foundation, Inc.
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

/* In various parts of this file we define the System V values for
   things as _SYSV_<whatever>.  Those are the values that System V
   uses for termio, and also (SVR4) termios.  Not necessarily the
   same as the GNU termios that the library user sees.  */

/* Number of elements of c_cc.  termio only.  */
#define _SYSV_NCC 8

#define _SYSV_VINTR 0
#define _SYSV_VQUIT 1
#define _SYSV_VERASE 2
#define _SYSV_VKILL 3
#define _SYSV_VEOF 4
/* This field means VEOF if ICANON, VMIN if not.  */
#define _SYSV_VMIN 4
#define _SYSV_VEOL 5
/* This field means VEOL if ICANON, VTIME if not.  */
#define _SYSV_VTIME 5
#define _SYSV_VEOL2 6

/* Flags in c_iflag.  */
#define _SYSV_IGNBRK 1
#define _SYSV_BRKINT 2
#define _SYSV_IGNPAR 4
#define _SYSV_PARMRK 8
#define _SYSV_INPCK 0x10
#define _SYSV_ISTRIP 0x20
#define _SYSV_INLCR 0x40
#define _SYSV_IGNCR 0x80
#define _SYSV_ICRNL 0x100
#define _SYSV_IUCLC 0x200
#define _SYSV_IXON 0x400
#define _SYSV_IXANY 0x800
#define _SYSV_IXOFF 0x1000
#define _SYSV_IMAXBEL 0x2000

/* Flags in c_cflag.  */
#define _SYSV_CBAUD 0xf
#define _SYSV_CIBAUD 0xf0000	/* termios only.  */
#define _SYSV_IBSHIFT 16
/* Values for CBAUD and CIBAUD.  */
#define _SYSV_B0 0
#define _SYSV_B50 1
#define _SYSV_B75 2
#define _SYSV_B110 3
#define _SYSV_B134 4
#define _SYSV_B150 5
#define _SYSV_B200 6
#define _SYSV_B300 7
#define _SYSV_B600 8
#define _SYSV_B1200 9
#define _SYSV_B1800 10
#define _SYSV_B2400 11
#define _SYSV_B4800 12
#define _SYSV_B9600 13
#define _SYSV_B19200 14
#define _SYSV_B38400 15

#define _SYSV_CS5 0
#define _SYSV_CS6 0x10
#define _SYSV_CS7 0x20
#define _SYSV_CS8 0x30
#define _SYSV_CSIZE 0x30
#define _SYSV_CSTOPB 0x40
#define _SYSV_CREAD 0x80
#define _SYSV_PARENB 0x100
#define _SYSV_PARODD 0x200
#define _SYSV_HUPCL 0x400
#define _SYSV_CLOCAL 0x800

/* Flags in c_lflag.  */
#define _SYSV_ISIG 1
#define _SYSV_ICANON 2
#define _SYSV_ECHO 8
#define _SYSV_ECHOE 0x10
#define _SYSV_ECHOK 0x20
#define _SYSV_ECHONL 0x40
#define _SYSV_NOFLSH 0x80
#define _SYSV_TOSTOP 0x100
#define _SYSV_ECHOCTL 0x200
#define _SYSV_ECHOPRT 0x400
#define _SYSV_ECHOKE 0x800
#define _SYSV_FLUSHO 0x2000
#define _SYSV_PENDIN 0x4000
#define _SYSV_IEXTEN 0x8000

/* Flags in c_oflag.  */
#define _SYSV_OPOST 1
#define _SYSV_OLCUC 2
#define _SYSV_ONLCR 4
#define _SYSV_NLDLY 0x100
#define _SYSV_NL0 0
#define _SYSV_NL1 0x100
#define _SYSV_CRDLY 0x600
#define _SYSV_CR0 0
#define _SYSV_CR1 0x200
#define _SYSV_CR2 0x400
#define _SYSV_CR3 0x600
#define _SYSV_TABDLY 0x1800
#define _SYSV_TAB0 0
#define _SYSV_TAB1 0x0800
#define _SYSV_TAB2 0x1000
/* TAB3 is an obsolete name for XTABS.  But we provide it since some
   programs expect it to exist.  */
#define _SYSV_TAB3 0x1800
#define _SYSV_XTABS 0x1800
#define _SYSV_BSDLY 0x2000
#define _SYSV_BS0 0
#define _SYSV_BS1 0x2000
#define _SYSV_VTDLY 0x4000
#define _SYSV_VT0 0
#define _SYSV_VT1 0x4000
#define _SYSV_FFDLY 0x8000
#define _SYSV_FF0 0
#define _SYSV_FF1 0x8000

/* ioctl's.  */

#define _TCGETA 0x5401
#define _TCSETA 0x5402
#define _TCSETAW 0x5403
#define _TCSETAF 0x5404
#define _TCSBRK 0x5405
#define _TCXONC 0x5406
#define _TCFLSH 0x5407
#define _TIOCGPGRP 0x7414
#define _TIOCSPGRP 0x7415

struct __sysv_termio
  {
    unsigned short c_iflag;
    unsigned short c_oflag;
    unsigned short c_cflag;
    unsigned short c_lflag;
    char c_line;
    unsigned char c_cc[_SYSV_NCC];
  };
