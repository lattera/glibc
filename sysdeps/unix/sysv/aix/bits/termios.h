/* termios type and macro definitions.  AIX version.
   Copyright (C) 1999, 2000 Free Software Foundation, Inc.
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

#ifndef _TERMIOS_H
# error "Never include <bits/termios.h> directly; use <termios.h> instead."
#endif

typedef unsigned char	cc_t;
typedef unsigned int	speed_t;
typedef unsigned int	tcflag_t;

#define NCCS 16
struct termios
  {
    tcflag_t c_iflag;		/* input mode flags */
    tcflag_t c_oflag;		/* output mode flags */
    tcflag_t c_cflag;		/* control mode flags */
    tcflag_t c_lflag;		/* local mode flags */
    cc_t c_cc[NCCS];		/* control characters */
  };

/* c_cc characters */
#define VINTR 0
#define VQUIT 1
#define VERASE 2
#define VKILL 3
#define VEOF 4
#define VEOL 5
#define VSTART 7
#define VSTOP 8
#define VSUSP 9
#define VMIN 4
#define VTIME 5
#define VEOL2 6
#define VDSUSP 10
#define VREPRINT 11
#define VDISCARD 12
#define VWERSE 13
#define VWERASE VWERSE
#define VLNEXT 14
#define VSTRT VSTART

/* c_iflag bits */
#define IGNBRK	0000001
#define BRKINT	0000002
#define IGNPAR	0000004
#define PARMRK	0000010
#define INPCK	0000020
#define ISTRIP	0000040
#define INLCR	0000100
#define IGNCR	0000200
#define ICRNL	0000400
#define IXON	0001000
#define IXOFF	0002000
#define IUCLC	0004000
#define IXANY	0010000
#define IMAXBEL	0200000

/* c_oflag bits */
#define OPOST	0000001
#define OLCUC	0000002
#define ONLCR	0000004
#define OCRNL	0000010
#define ONOCR	0000020
#define ONLRET	0000040
#define OFILL	0000100
#define OFDEL	0000200
#if defined __USE_MISC || defined __USE_XOPEN
# define CRDLY	0001400
# define   CR0	0000000
# define   CR1	0000400
# define   CR2	0001000
# define   CR3	0001400
# define TABDLY	0006000
# define   TAB0	0000000
# define   TAB1	0002000
# define   TAB2	0004000
# define   TAB3	0006000
# define BSDLY	0010000
# define   BS0	0000000
# define   BS1	0010000
# define FFDLY	0020000
# define   FF0	0000000
# define   FF1	0020000
# define NLDLY	0040000
# define   NL0	0000000
# define   NL1	0040000
#endif

#define VTDLY	0100000
#define   VT0	0000000
#define   VT1	0100000

/* c_cflag bit meaning */
#ifdef __USE_MISC
# define CBAUD	0000017
#endif
#define  B0	0000000		/* hang up */
#define  B50	0000001
#define  B75	0000002
#define  B110	0000003
#define  B134	0000004
#define  B150	0000005
#define  B200	0000006
#define  B300	0000007
#define  B600	0000010
#define  B1200	0000011
#define  B1800	0000012
#define  B2400	0000013
#define  B4800	0000014
#define  B9600	0000015
#define  B19200	0000016
#define  B38400	0000017
#ifdef __USE_MISC
# define EXTA B19200
# define EXTB B38400
#endif
#define CSIZE	0000060
#define   CS5	0000000
#define   CS6	0000020
#define   CS7	0000040
#define   CS8	0000060
#define CSTOPB	0000100
#define CREAD	0000200
#define PARENB	0000400
#define PARODD	0001000
#define HUPCL	0002000
#define CLOCAL	0004000
#ifdef __USE_MISC
# define CIBAUD	  000003600000		/* input baud rate (not used) */
# define CRTSCTS  020000000000		/* flow control */
#endif

/* c_lflag bits */
#define ISIG	0000001
#define ICANON	0000002
#if defined __USE_MISC || defined __USE_XOPEN
# define XCASE	0000004
#endif
#define ECHO	0000010
#define ECHOE	0000020
#define ECHOK	0000040
#define ECHONL	0000100
#define NOFLSH	0000200
#define TOSTOP	0200000
#ifdef __USE_MISC
# define ECHOCTL 0400000
# define ECHOPRT 01000000
# define ECHOKE	 02000000
# define FLUSHO	 004000000
# define PENDIN	 04000000000
#endif
#define IEXTEN	010000000

/* tcflow() and TCXONC use these */
#define	TCOOFF		0
#define	TCOON		1
#define	TCIOFF		2
#define	TCION		3

/* tcflush() and TCFLSH use these */
#define	TCIFLUSH	0
#define	TCOFLUSH	1
#define	TCIOFLUSH	2

/* tcsetattr uses these */
#define	TCSANOW		0
#define	TCSADRAIN	1
#define	TCSAFLUSH	2


#define _IOT_termios /* Hurd ioctl type field.  */ \
  _IOT (_IOTS (cflag_t), 4, _IOTS (cc_t), NCCS, _IOTS (speed_t), 2)
