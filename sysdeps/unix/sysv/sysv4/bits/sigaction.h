/* The proper definitions for SVR4's sigaction.
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

#ifndef _SIGNAL_H
# error "Never include <bits/sigaction.h> directly; use <signal.h> instead."
#endif

/* Structure describing the action to be taken when a signal arrives.  */
struct sigaction
  {
    /* Special flags.  */
    int sa_flags;

    /* Signal handler.  */
    __sighandler_t sa_handler;

    /* Additional set of signals to be blocked.  */
    __sigset_t sa_mask;

    /* Padding.  */
    int sa_resv[2];
  };

/* Bits in `sa_flags'.  */
#ifdef __USE_MISC
# define SA_ONSTACK	0x1	/* Take signal on signal stack.  */
# define SA_RESETHAND	0x2	/* Reset to SIG_DFL on entry to handler.  */
# define SA_RESTART	0x4	/* Restart syscall on signal return.  */
# define SA_SIGINFO	0x8	/* Provide additional info to the handler.  */
# define SA_NODEFER	0x10	/* Don't automatically block the signal when
				   its handler is being executed.  */
# define SA_NOCLDWAIT	0x10000	/* Don't save zombie processes.  */
#endif
#define	SA_NOCLDSTOP	0x20000	/* Don't send SIGCHLD when children stop.  */

/* Values for the HOW argument to `sigprocmask'.  */
#define	SIG_BLOCK	1	/* Block signals.  */
#define	SIG_UNBLOCK	2	/* Unblock signals.  */
#define	SIG_SETMASK	3	/* Set the set of blocked signals.  */
