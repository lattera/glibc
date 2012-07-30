/* Signal number constants.  Generic version.
   Copyright (C) 1991-2012 Free Software Foundation, Inc.
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

#ifdef	_SIGNAL_H

/* Fake signal functions.  */

#define	SIG_ERR	 ((__sighandler_t) -1)	/* Error return.  */
#define	SIG_DFL	 ((__sighandler_t)  0)	/* Default action.  */
#define	SIG_IGN	 ((__sighandler_t)  1)	/* Ignore signal.  */

#ifdef __USE_UNIX98
# define SIG_HOLD ((__sighandler_t)  2)	/* Add signal to hold mask.  */
#endif

/* We define here all the signal names listed in POSIX (1003.1-2008).
   Signals in the 1-15 range are defined with their historical numbers.
   For other signals specified by POSIX, we use the BSD numbers.  */

/* ISO C99 signals.  */
#define	SIGINT		2	/* Interactive attention signal.  */
#define	SIGILL		4	/* Illegal instruction.  */
#define	SIGABRT		6	/* Abnormal termination.  */
#define	SIGFPE		8	/* Erroneous arithmetic operation.  */
#define	SIGSEGV		11	/* Invalid access to storage.  */
#define	SIGTERM		15	/* Termination request.  */

/* Historical signals specified by POSIX. */
#define	SIGHUP		1	/* Hangup.  */
#define	SIGQUIT		3	/* Quit.  */
#define	SIGTRAP		5	/* Trace/breakpoint trap.  */
#define	SIGKILL		9	/* Killed.  */
#define SIGBUS		10	/* Bus error.  */
#define	SIGSYS		12	/* Bad system call.  */
#define	SIGPIPE		13	/* Broken pipe.  */
#define	SIGALRM		14	/* Alarm clock.  */

/* New(er) POSIX signals (1003.1-2008).  */
#define	SIGURG		16 /* High bandwidth data is available at a socket.  */
#define	SIGSTOP		17	/* Stopped (signal).  */
#define	SIGTSTP		18	/* Stopped.  */
#define	SIGCONT		19	/* Continued.  */
#define	SIGCHLD		20	/* Child terminated or stopped.  */
#define	SIGTTIN		21	/* Background read from control terminal.  */
#define	SIGTTOU		22	/* Background write to control terminal.  */
#define	SIGPOLL 	23	/* Pollable event occurred (System V).  */
#define	SIGIO		SIGPOLL /* I/O now possible (4.2 BSD).  */
#define	SIGXCPU		24	/* CPU time limit exceeded.  */
#define	SIGXFSZ		25	/* File size limit exceeded.  */
#define	SIGVTALRM	26	/* Virtual timer expired.  */
#define	SIGPROF		27	/* Profiling timer expired.  */
#define	SIGUSR1		30	/* User-defined signal 1.  */
#define	SIGUSR2		31	/* User-defined signal 2.  */

#define	_NSIG		32

/* Archaic names for compatibility. */
#define	SIGIOT		SIGABRT	/* IOT instruction, abort() on a PDP11 */
#define	SIGCLD		SIGCHLD	/* Old System V name */

#endif	/* <signal.h> included.  */
