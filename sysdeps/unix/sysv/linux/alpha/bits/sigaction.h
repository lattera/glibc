/* The proper definitions for Linux/Alpha sigaction.
   Copyright (C) 1996, 1997 Free Software Foundation, Inc.
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
    unsigned int sa_flags;
  };

/* Bits in `sa_flags'.  */
#define	SA_NOCLDSTOP  0x00000004 /* Don't send SIGCHLD when children stop.  */
#define SA_SIGINFO    0x00000040 /* Invoke signal-catching function with three
				    arguments instead of one. */
#ifdef __USE_MISC
# define SA_ONSTACK   0x00000001 /* Use signal stack by using `sa_restorer'. */
# define SA_RESTART   0x00000002 /* Restart syscall on signal return.  */
# define SA_INTERRUPT 0x20000000 /* Historical no-op.  */
# define SA_NOMASK    0x00000008 /* Don't automatically block the signal
				    when its handler is being executed.  */
# define SA_ONESHOT   0x00000010 /* Reset to SIG_DFL on entry to handler.  */

/* Some aliases for the SA_ constants.  */
# define SA_NODEFER   SA_NOMASK
# define SA_RESETHAND SA_ONESHOT
# define SA_STACK     SA_ONSTACK
#endif

/* Values for the HOW argument to `sigprocmask'.  */
#define	SIG_BLOCK     1		 /* Block signals.  */
#define	SIG_UNBLOCK   2		 /* Unblock signals.  */
#define	SIG_SETMASK   3		 /* Set the set of blocked signals.  */
