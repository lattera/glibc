/* termios type and macro definitions.  SunOS 4 version.
   Copyright (C) 1993, 1994, 1996, 1997 Free Software Foundation, Inc.
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

/* Type of terminal control flag masks.  */
typedef unsigned long int tcflag_t;

/* Type of control characters.  */
typedef unsigned char cc_t;

/* Type of baud rate specifiers.  */
typedef unsigned int speed_t;

/* Terminal control structure.  */
struct termios
{
  /* Input modes.  */
  tcflag_t c_iflag;
#define	IGNBRK	0x0001		/* Ignore break condition.  */
#define	BRKINT	0x0002		/* Signal interrupt on break.  */
#define	IGNPAR	0x0004		/* Ignore characters with parity errors.  */
#define	PARMRK	0x0008		/* Mark parity and framing errors.  */
#define	INPCK	0x0010		/* Enable input parity check.  */
#define	ISTRIP	0x0020		/* Strip 8th bit off characters.  */
#define	INLCR	0x0040		/* Map NL to CR on input.  */
#define	IGNCR	0x0080		/* Ignore CR.  */
#define	ICRNL	0x0100		/* Map CR to NL on input.  */
#ifdef __USE_BSD
# define IUCLC	0x0200		/* Map upper case to lower case on input.  */
#endif
#define	IXON	0x0400		/* Enable start/stop output control.  */
#define	IXOFF	0x1000		/* Enable start/stop input control.  */
#ifdef	__USE_BSD
# define IXANY	0x0800		/* Any character will restart after stop.  */
# define IMAXBEL	0x2000		/* Ring bell when input queue is full.  */
#endif

  /* Output modes.  */
  tcflag_t c_oflag;
#define	OPOST	0x0001		/* Perform output processing.  */
#ifdef	__USE_BSD
# define OLCUC	0x00000002	/* Map lower case to upper case on output.  */
# define ONLCR	0x00000004	/* Map NL to CR-NL on output.  */
# define OCRNL	0x00000008
# define ONOCR	0x00000010
# define ONLRET	0x00000020
# define OFILL	0x00000040
# define OFDEL	0x00000080
# define NLDLY	0x00000100
# define NL0	0
# define NL1	0x00000100
# define CRDLY	0x00000600
# define CR0	0
# define CR1	0x00000200
# define CR2	0x00000400
# define CR3	0x00000600
# define TABDLY	0x00001800
# define TAB0	0
# define TAB1	0x00000800
# define TAB2	0x00001000
# define XTABS	0x00001800
# define TAB3	XTABS
# define BSDLY	0x00002000
# define BS0	0
# define BS1	0x00002000
# define VTDLY	0x00004000
# define VT0	0
# define VT1	0x00004000
# define FFDLY	0x00008000
# define FF0	0
# define FF1	0x00008000
# define PAGEOUT 0x00010000
# define WRAP	0x00020000
#endif

  /* Control modes.  */
  tcflag_t c_cflag;
#define	CSIZE	(CS5|CS6|CS7|CS8) /* Number of bits per byte (mask).  */
#define	CS5	0		/* 5 bits per byte.  */
#define	CS6	0x00000010	/* 6 bits per byte.  */
#define	CS7	0x00000020	/* 7 bits per byte.  */
#define	CS8	0x00000030	/* 8 bits per byte.  */
#define	CSTOPB	0x00000040	/* Two stop bits instead of one.  */
#define	CREAD	0x00000080	/* Enable receiver.  */
#define	PARENB	0x00000100	/* Parity enable.  */
#define	PARODD	0x00000200	/* Odd parity instead of even.  */
#define	HUPCL	0x00000400	/* Hang up on last close.  */
#define	CLOCAL	0x00000800	/* Ignore modem status lines.  */
#ifdef	__USE_BSD
# define LOBLK	0x00001000
# define CRTSCTS	0x80000000
# define CIBAUD	0x000f0000	/* Mask for input speed from c_cflag.  */
# define CBAUD	0x0000000f	/* Mask for output speed from c_cflag.  */
# define IBSHIFT	16		/* Bits to shift for input speed.  */
#endif

  /* Input and output baud rates.  These are encoded in c_cflag.  */
#define B0      0
#define B50     1
#define B75     2
#define B110    3
#define B134    4
#define B150    5
#define B200    6
#define B300    7
#define B600    8
#define B1200   9
#define B1800   10
#define B2400   11
#define B4800   12
#define B9600   13
#define B19200  14
#define B38400  15
#ifdef __USE_BSD
# define EXTA   14
# define EXTB   15
#endif

  /* Local modes.  */
  tcflag_t c_lflag;
#ifdef	__USE_BSD
# define ECHOKE	0x00000800	/* Visual erase for KILL.  */
#endif
#define	ECHOE	0x00000010	/* Visual erase for ERASE.  */
#define	ECHOK	0x00000020	/* Echo NL after KILL.  */
#define	ECHO	0x00000008	/* Enable echo.  */
#define	ECHONL	0x00000040	/* Echo NL even if ECHO is off.  */
#ifdef	__USE_BSD
# define ECHOPRT	0x00000400	/* Hardcopy visual erase.  */
# define ECHOCTL	0x00000200	/* Echo control characters as ^X.  */
#endif
#define	ISIG	0x00000001	/* Enable signals.  */
#define	ICANON	0x00000002	/* Do erase and kill processing.  */
#define	IEXTEN	0x00008000	/* Enable DISCARD and LNEXT.  */
#define	TOSTOP	0x00000100	/* Send SIGTTOU for background output.  */
#ifdef	__USE_BSD
# define PENDIN	0x00004000	/* Retype pending input (state).  */
#endif
#define	NOFLSH	0x00000080	/* Disable flush after interrupt.  */

  char c_line;			/* Line discipline (?) */

  /* Control characters.  */
#define	VEOF	4		/* End-of-file character [ICANON].  */
#define	VEOL	5		/* End-of-line character [ICANON].  */
#ifdef	__USE_BSD
# define VEOL2	6		/* Second EOL character [ICANON].  */
# define VSWTCH	7		/* ??? */
#endif
#define	VERASE	2		/* Erase character [ICANON].  */
#ifdef	__USE_BSD
# define VWERASE	14		/* Word-erase character [ICANON].  */
#endif
#define	VKILL	3		/* Kill-line character [ICANON].  */
#ifdef	__USE_BSD
# define VREPRINT 12		/* Reprint-line character [ICANON].  */
#endif
#define	VINTR	0		/* Interrupt character [ISIG].  */
#define	VQUIT	1		/* Quit character [ISIG].  */
#define	VSUSP	10		/* Suspend character [ISIG].  */
#ifdef	__USE_BSD
# define VDSUSP	11		/* Delayed suspend character [ISIG].  */
#endif
#define	VSTART	8		/* Start (X-ON) character [IXON, IXOFF].  */
#define	VSTOP	9		/* Stop (X-OFF) character [IXON, IXOFF].  */
#ifdef	__USE_BSD
# define VLNEXT	15		/* Literal-next character [IEXTEN].  */
# define VDISCARD 13		/* Discard character [IEXTEN].  */
#endif
#define	VMIN	VEOF		/* Minimum number of bytes read at once [!ICANON].  */
#define	VTIME	VEOL		/* Time-out value (tenths of a second) [!ICANON].  */
#define	NCCS	17
  cc_t c_cc[NCCS];
};

#define _IOT_termios /* Hurd ioctl type field.  */ \
  _IOT (_IOTS (cflag_t), 4, _IOTS (cc_t), NCCS, _IOTS (speed_t), 2)

/* Values for the OPTIONAL_ACTIONS argument to `tcsetattr'.  */
#define	TCSANOW		0	/* Change immediately.  */
#define	TCSADRAIN	1	/* Change when pending output is written.  */
#define	TCSAFLUSH	2	/* Flush pending input before changing.  */

/* Values for the QUEUE_SELECTOR argument to `tcflush'.  */
#define	TCIFLUSH	0	/* Discard data received but not yet read.  */
#define	TCOFLUSH	1	/* Discard data written but not yet sent.  */
#define	TCIOFLUSH	2	/* Discard all pending data.  */

/* Values for the ACTION argument to `tcflow'.  */
#define	TCOOFF	0		/* Suspend output.  */
#define	TCOON	1		/* Restart suspended output.  */
#define	TCIOFF	2		/* Send a STOP character.  */
#define	TCION	3		/* Send a START character.  */
