/* Copyright (C) 1992, 1993 Free Software Foundation, Inc.
   Contributed by Brendan Kehoe (brendan@zen.org).

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#ifdef	_SIGNAL_H

/* This file defines the fake signal functions and signal
   number constants for System V Release 4 UNIX.  */

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
#define	SIGWINCH	20	/* Window size change.  */
#define	SIGURG		21	/* Urgent condition on socket.*/
#define	SIGIO		SIGPOLL	/* I/O now possible.  */
#define	SIGPOLL		22	/* Same as SIGIO? (SVID).  */
#define	SIGSTOP		23	/* Stop, unblockable (POSIX).  */
#define	SIGTSTP		24	/* Keyboard stop (POSIX).  */
#define	SIGCONT		25	/* Continue (POSIX).  */
#define	SIGTTIN		26	/* Background read from tty (POSIX).  */
#define	SIGTTOU		27	/* Background write to tty (POSIX).  */
#define	SIGVTALRM	28	/* Virtual alarm clock.  */
#define	SIGPROF		29	/* Profiling alarm clock.  */
#define	SIGXCPU		30	/* CPU limit exceeded.  */
#define	SIGXFSZ		31	/* File size limit exceeded.  */

#endif	/* <signal.h> included.  */

#define	_NSIG		32	/* Biggest signal number + 1.  */
