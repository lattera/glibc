/* Copyright (C) 1991, 92, 93, 94, 96, 97, 98 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/*
 *	POSIX Standard: 7.1-2 General Terminal Interface	<termios.h>
 */

#ifndef	_TERMIOS_H
#define	_TERMIOS_H	1

#include <features.h>
#ifdef __USE_UNIX98
/* We need `pid_t'.  */
# include <bits/types.h>
# ifndef pid_t
typedef __pid_t pid_t;
#  define pid_t pid_t
# endif
#endif

__BEGIN_DECLS

/* Get the system-dependent definitions of `struct termios', `tcflag_t',
   `cc_t', `speed_t', and all the macros specifying the flag bits.  */
#include <bits/termios.h>

#ifdef __USE_BSD
/* Compare a character C to a value VAL from the `c_cc' array in a
   `struct termios'.  If VAL is _POSIX_VDISABLE, no character can match it.  */
# define CCEQ(val, c)	((c) == (val) && (val) != _POSIX_VDISABLE)
#endif

/* Return the output baud rate stored in *TERMIOS_P.  */
extern speed_t cfgetospeed __P ((__const struct termios *__termios_p));

/* Return the input baud rate stored in *TERMIOS_P.  */
extern speed_t cfgetispeed __P ((__const struct termios *__termios_p));

/* Set the output baud rate stored in *TERMIOS_P to SPEED.  */
extern int cfsetospeed __P ((struct termios *__termios_p, speed_t __speed));

/* Set the input baud rate stored in *TERMIOS_P to SPEED.  */
extern int cfsetispeed __P ((struct termios *__termios_p, speed_t __speed));

#ifdef	__USE_BSD
/* Set both the input and output baud rates in *TERMIOS_OP to SPEED.  */
extern int cfsetspeed __P ((struct termios *__termios_p, speed_t __speed));
#endif


/* Put the state of FD into *TERMIOS_P.  */
extern int tcgetattr __P ((int __fd, struct termios *__termios_p));

/* Set the state of FD to *TERMIOS_P.
   Values for OPTIONAL_ACTIONS (TCSA*) are in <bits/termios.h>.  */
extern int tcsetattr __P ((int __fd, int __optional_actions,
			   __const struct termios *__termios_p));


#ifdef	__USE_BSD
/* Set *TERMIOS_P to indicate raw mode.  */
extern void cfmakeraw __P ((struct termios *__termios_p));
#endif

/* Send zero bits on FD.  */
extern int tcsendbreak __P ((int __fd, int __duration));

/* Wait for pending output to be written on FD.  */
extern int tcdrain __P ((int __fd));

/* Flush pending data on FD.
   Values for QUEUE_SELECTOR (TC{I,O,IO}FLUSH) are in <bits/termios.h>.  */
extern int tcflush __P ((int __fd, int __queue_selector));

/* Suspend or restart transmission on FD.
   Values for ACTION (TC[IO]{OFF,ON}) are in <bits/termios.h>.  */
extern int tcflow __P ((int __fd, int __action));


#ifdef __USE_UNIX98
/* Get process group ID for session leader for controlling terminal FD.  */
extern __pid_t tcgetsid __P ((int __fd));
#endif


#ifdef __USE_BSD
# include <sys/ttydefaults.h>
#endif

__END_DECLS

#endif /* termios.h  */
