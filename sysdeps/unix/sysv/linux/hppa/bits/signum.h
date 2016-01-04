/* Signal number definitions.  Linux/HPPA version.
   Copyright (C) 1995-2016 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#ifdef	_SIGNAL_H

/* Fake signal functions.  */
#define SIG_ERR	((__sighandler_t) -1)		/* Error return.  */
#define SIG_DFL	((__sighandler_t) 0)		/* Default action.  */
#define SIG_IGN	((__sighandler_t) 1)		/* Ignore signal.  */

#ifdef __USE_UNIX98
# define SIG_HOLD	((__sighandler_t) 2)	/* Add signal to hold mask.  */
#endif


/* Signals.  */
#define	SIGHUP		1	/* Hangup (POSIX).  */
#define	SIGINT		2	/* Interrupt (ANSI).  */
#define	SIGQUIT		3	/* Quit (POSIX).  */
#define	SIGILL		4	/* Illegal instruction (ANSI).  */
#define	SIGTRAP		5	/* Trace trap (POSIX).  */
#define	SIGABRT		6	/* Abort (ANSI).  */
#define	SIGIOT		6	/* IOT trap (4.2 BSD).  */
#define	SIGSTKFLT	7	/* Stack fault.  */
#define	SIGFPE		8	/* Floating-point exception (ANSI).  */
#define	SIGKILL		9	/* Kill, unblockable (POSIX).  */
#define	SIGBUS		10	/* BUS error (4.2 BSD).  */
#define	SIGSEGV		11	/* Segmentation violation (ANSI).  */
#define	SIGXCPU		12	/* CPU limit exceeded (4.2 BSD).  */
#define	SIGPIPE		13	/* Broken pipe (POSIX).  */
#define	SIGALRM		14	/* Alarm clock (POSIX).  */
#define	SIGTERM		15	/* Termination (ANSI).  */
#define	SIGUSR1		16	/* User-defined signal 1 (POSIX).  */
#define SIGUSR2		17	/* User-defined signal 2 (POSIX).  */
#define	SIGCLD		SIGCHLD	/* Same as SIGCHLD (System V).  */
#define	SIGCHLD		18	/* Child status has changed (POSIX).  */
#define	SIGPWR		19	/* Power failure restart (System V).  */
#define	SIGVTALRM	20	/* Virtual alarm clock (4.2 BSD).  */
#define	SIGPROF		21	/* Profiling alarm clock (4.2 BSD).  */
#define	SIGPOLL		SIGIO	/* Pollable event occurred (System V).  */
#define	SIGIO		22	/* I/O now possible (4.2 BSD).  */
#define	SIGWINCH	23	/* Window size change (4.3 BSD, Sun).  */
#define	SIGSTOP		24	/* Stop, unblockable (POSIX).  */
#define	SIGTSTP		25	/* Keyboard stop (POSIX).  */
#define	SIGCONT		26	/* Continue (POSIX).  */
#define	SIGTTIN		27	/* Background read from tty (POSIX).  */
#define	SIGTTOU		28	/* Background write to tty (POSIX).  */
#define	SIGURG		29	/* Urgent condition on socket (4.2 BSD).  */
#define	SIGXFSZ		30	/* File size limit exceeded (4.2 BSD).  */
#define SIGSYS		31	/* Bad system call.  */
#define SIGUNUSED	31

#define	_NSIG		65	/* Biggest signal number + 1
				   (including real-time signals).  */

#define SIGRTMIN        (__libc_current_sigrtmin ())
#define SIGRTMAX        (__libc_current_sigrtmax ())

/* These are the hard limits of the kernel.  These values should not be
   used directly at user level.  */
/* In the Linux kernel version 3.17, and glibc 2.21, the signal numbers
   were rearranged in order to make hppa like every other arch. Previously
   we started __SIGRTMIN at 37, and that meant several pieces of important
   software, including systemd, would fail to build. To support systemd we
   removed SIGEMT and SIGLOST, and rearranged the others according to
   expected values. This is technically an ABI incompatible change, but
   because zero applications use SIGSTKFLT, SIGXCPU, SIGXFSZ and SIGSYS
   nothing broke.  Nothing uses SIGEMT and SIGLOST, and they were present
   for HPUX compatibility which is no longer supported.  Thus because
   nothing breaks we don't do any compatibility work here.  */
#define __SIGRTMIN	32	/* Kernel > 3.17.  */
#define __SIGRTMAX	(_NSIG - 1)

#endif	/* <signal.h> included.  */
