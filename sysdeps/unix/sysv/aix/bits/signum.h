/* Signal number definitions.  AIX version.
   Copyright (C) 1995-1999, 2000 Free Software Foundation, Inc.
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

/* Fake signal functions.  */
#define SIG_ERR	((__sighandler_t) -1)		/* Error return.  */
#define SIG_DFL	((__sighandler_t) 0)		/* Default action.  */
#define SIG_IGN	((__sighandler_t) 1)		/* Ignore signal.  */

#ifdef __USE_UNIX98
# define SIG_HOLD	((__sighandler_t) 2)	/* Add signal to hold mask.  */
# define SIG_CATCHE	((__sighandler_t) 3)
#endif


/* Signals.  */
#define	SIGHUP		1	/* Hangup (POSIX).  */
#define	SIGINT		2	/* Interrupt (ANSI).  */
#define	SIGQUIT		3	/* Quit (POSIX).  */
#define	SIGILL		4	/* Illegal instruction (ANSI).  */
#define	SIGTRAP		5	/* Trace trap (POSIX).  */
#define	SIGABRT		6	/* Abort (ANSI).  */
#define SIGIOT		SIGABRT	/* Abort (terminal) process.  */
#define SIGEMT		7	/* EMT instruction.  */
#define	SIGFPE		8	/* Floating-point exception (ANSI).  */
#define	SIGKILL		9	/* Kill, unblockable (POSIX).  */
#define	SIGBUS		10	/* BUS error (4.2 BSD).  */
#define	SIGSEGV		11	/* Segmentation violation (ANSI).  */
#define SIGSYS		12	/* Bad system call.  */
#define	SIGPIPE		13	/* Broken pipe (POSIX).  */
#define	SIGALRM		14	/* Alarm clock (POSIX).  */
#define	SIGTERM		15	/* Termination (ANSI).  */
#define	SIGURG		16	/* Urgent condition on socket (4.2 BSD).  */
#define SIGIOINT	SIGURG	/* Printer to backend error signal.  */
#define	SIGSTOP		17	/* Stop, unblockable (POSIX).  */
#define	SIGTSTP		18	/* Keyboard stop (POSIX).  */
#define	SIGCONT		19	/* Continue (POSIX).  */
#define	SIGCLD		SIGCHLD	/* Same as SIGCHLD (System V).  */
#define	SIGCHLD		20	/* Child status has changed (POSIX).  */
#define	SIGTTIN		21	/* Background read from tty (POSIX).  */
#define	SIGTTOU		22	/* Background write to tty (POSIX).  */
#define	SIGIO		23	/* I/O now possible (4.2 BSD).  */
#define SIGAIO		SIGIO	/* Base LAN I/O.  */
#define SIGPTY		SIGIO	/* PTY I/O.  */
#define SIGPOLL		SIGIO	/* ANother I/O event.  */
#define	SIGXCPU		24	/* CPU limit exceeded (4.2 BSD).  */
#define	SIGXFSZ		25	/* File size limit exceeded (4.2 BSD).  */
#define SIGMSG		27	/* Input data is in the ring buffer.  */
#define	SIGWINCH	28	/* Window size change (4.3 BSD, Sun).  */
#define	SIGPWR		29	/* Power failure restart (System V).  */
#define	SIGUSR1		30	/* User-defined signal 1 (POSIX).  */
#define	SIGUSR2		31	/* User-defined signal 2 (POSIX).  */
#define	SIGPROF		32	/* Profiling alarm clock (4.2 BSD).  */
#define SIGDANGER	33	/* System crash imminent.  */
#define	SIGVTALRM	34	/* Virtual alarm clock (4.2 BSD).  */
#define SIGMIGRATE	35	/* Migrate process.  */
#define SIGPRE		36	/* Programming exception.  */
#define SIGVIRT		37	/* AIX virtual time alarm.  */
#define SIGARLM1	38	/* Reserved, don't use.  */
#define SIGWAITING	39	/* Reserved, don't use.  */
#define SIGCPUFAIL	59	/* Predictive de-configuration of processors.*/
#define SIGKAP		60	/* Keep alive poll from native keyboard.  */
#define SIGGRANT	SIGKAP	/* Monitor mode granted.  */
#define SIGRETRACT	61	/* Monitor mode should be relinguished.  */
#define SIGSOUND	62	/* Sound control has completed.  */
#define SIGSAK		63	/* Secure attentation key.  */

#define	_NSIG		64	/* Biggest signal number + 1
				   (including real-time signals).  */

#define SIGRTMIN        (__libc_current_sigrtmin ())
#define SIGRTMAX        (__libc_current_sigrtmax ())

/* These are the hard limits of the kernel.  These values should not be
   used directly at user level.  */
#define __SIGRTMIN	888
#define __SIGRTMAX	999

#endif	/* <signal.h> included.  */
