/* Signal number definitions.  Irix4 version.
   Copyright (C) 1994, 1996 Free Software Foundation, Inc.
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

#ifdef	_SIGNAL_H

/* This file defines the fake signal functions and signal
   number constants for SGI Irix 4.  */

/* Fake signal functions.  */
#define	SIG_ERR	((__sighandler_t) -1)
#define	SIG_DFL	((__sighandler_t) 0)
#define	SIG_IGN	((__sighandler_t) 1)


/* Signals.  */
#define	SIGHUP		1	/* Hangup (POSIX).  */
#define	SIGINT		2	/* Interrupt (ANSI).  */
#define	SIGQUIT		3	/* Quit (POSIX).  */
#define	SIGILL		4	/* Illegal instruction (ANSI).  */
#define	SIGABRT		SIGIOT	/* Abort (ANSI).  */
#define	SIGTRAP		5	/* Trace trap (POSIX).  */
#define	SIGIOT		6	/* IOT trap.  */
#define	SIGEMT		7	/* EMT trap.  */
#define	SIGFPE		8	/* Floating-point exception (ANSI).  */
#define	SIGKILL		9	/* Kill, unblockable (POSIX).  */
#define	SIGBUS		10	/* Bus error.  */
#define	SIGSEGV		11	/* Segmentation violation (ANSI).  */
#define	SIGSYS		12	/* Bad argument to system call*/
#define	SIGPIPE		13	/* Broken pipe (POSIX).  */
#define	SIGALRM		14	/* Alarm clock (POSIX).  */
#define	SIGTERM		15	/* Termination (ANSI).  */
#define	SIGUSR1		16	/* User-defined signal 1 (POSIX).  */
#define	SIGUSR2		17	/* User-defined signal 2 (POSIX).  */
#define	SIGCHLD		18	/* Child status has changed (POSIX).  */
#define	SIGCLD		SIGCHLD	/* Same as SIGCHLD (System V).  */
#define SIGPWR		19	/* Power going down.  */
#define	SIGSTOP		20	/* Stop, unblockable (POSIX).  */
#define	SIGTSTP		21	/* Keyboard stop (POSIX).  */
#define	SIGPOLL		22	/* Same as SIGIO? (SVID).  */
#define	SIGIO		23	/* I/O now possible.  */
#define	SIGURG		24	/* Urgent condition on socket.*/
#define	SIGWINCH	25	/* Window size change.  */
#define	SIGVTALRM	26	/* Virtual alarm clock.  */
#define	SIGPROF		27	/* Profiling alarm clock.  */
#define	SIGCONT		28	/* Continue (POSIX).  */
#define	SIGTTIN		29	/* Background read from tty (POSIX).  */
#define	SIGTTOU		30	/* Background write to tty (POSIX).  */
#define	SIGXCPU		31	/* CPU limit exceeded.  */
#define	SIGXFSZ		32	/* File size limit exceeded.  */

#endif	/* <signal.h> included.  */

#define	_NSIG		33	/* Biggest signal number + 1.  */
