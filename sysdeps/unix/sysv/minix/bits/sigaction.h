/* Copyright (C) 1992, 1996, 1997 Free Software Foundation, Inc.
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
    /* Signal handler.  */
    __sighandler_t sa_handler;

    /* Additional set of signals to be blocked.  */
    __sigset_t sa_mask;

    /* Special flags.  */
    int sa_flags;
  };

/* Bits in `sa_flags'.  */
#ifdef	__USE_MISC
# define SA_ONSTACK	0x1	/* Take signal on signal stack.  */
# define SA_RESETHAND	0x2	/* Reset signal handler when signal caught.  */
# define SA_NODEFER	0x4	/* Don't block signal while catching it.  */
# define SA_RESTART	0x8	/* Restart syscall on signal return.  */
# define SA_SIGINFO	0x10	/* Extended signal handling.  */
# define SA_NOCLDWAIT	0x20	/* Don't create zombies.  */
# define SA_COMPAT	0x80	/* Internal flag for old signal catchers.  */
# define SA_DISABLE	0x100	/* Disable alternate signal stack.  */
#endif
#define	SA_NOCLDSTOP	0x40	/* Don't send SIGCHLD when children stop.  */


/* Values for the HOW argument to `sigprocmask'.  */
#define	SIG_BLOCK	0	/* Block signals.  */
#define	SIG_UNBLOCK	1	/* Unblock signals.  */
#define	SIG_SETMASK	2	/* Set the set of blocked signals.  */
