/* Copyright (C) 1991, 1993, 1996, 1998 Free Software Foundation, Inc.
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

/* Signals in the 1-15 range are defined with their historical numbers.
   Signals in the 20-25 range are relatively new and have no ingrained
   numbers. */

/* ANSI signals.  */
#define	SIGINT	2	/* Interactive attention signal.  */
#define	SIGILL	4	/* Illegal instruction.  */
#define	SIGABRT	6	/* Abnormal termination.  */
#define	SIGFPE	8	/* Erroneous arithmetic operation.  */
#define	SIGSEGV	11	/* Invalid access to storage.  */
#define	SIGTERM	15	/* Termination request.  */

/* Historical signals specified by POSIX. */
#define	SIGHUP	1	/* Hangup.  */
#define	SIGQUIT	3	/* Quit.  */
#define	SIGKILL	9	/* Kill (cannot be blocked, caught, or ignored).  */
#define	SIGPIPE	13	/* Broken pipe.  */
#define	SIGALRM	14	/* Alarm clock.  */

/* New(er) POSIX signals. */
#define	SIGSTOP	20	/* Stop (cannot be blocked, caught, or ignored).  */
#define	SIGCONT	21	/* Continue.  */
#define	SIGTSTP	22	/* Keyboard stop.  */
#define	SIGTTIN	23	/* Background read from control terminal.  */
#define	SIGTTOU	24	/* Background write to control terminal.  */
#define	SIGCHLD	25	/* Child terminated or stopped.  */

#define	_NSIG	26

/* Archaic names for compatibility. */
#define	SIGIOT  SIGABRT	/* IOT instruction, abort() on a PDP11 */
#define	SIGCLD  SIGCHLD	/* Old System V name */

#endif	/* <signal.h> included.  */
