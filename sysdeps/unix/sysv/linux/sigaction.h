/* The proper definitions for Linux's sigaction.
Copyright (C) 1993, 1994, 1995 Free Software Foundation, Inc.
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
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

/* Structure describing the action to be taken when a signal arrives.  */
struct sigaction
  {
    /* Signal handler.  */
    __sighandler_t sa_handler;

    /* Additional set of signals to be blocked.  */
    __sigset_t sa_mask;

    /* Special flags.  */
    unsigned long sa_flags;

    /* Restore handler.  */
    void (*sa_restorer) __P ((void));
  };

/* Bits in `sa_flags'.  */
#define	SA_NOCLDSTOP 1		/* Don't send SIGCHLD when children stop.  */
#ifdef __USE_MISC
#define SA_STACK     0x08000000	/* Use signal stack by using `sa_restorer'.  */
#define SA_RESTART   0x10000000	/* Don't restart syscall on signal return.  */
#define SA_INTERRUPT 0x20000000	/* Historical no-op.  */
#define SA_NODEFER   0x40000000	/* Don't automatically block the signal when
				   its handler is being executed.  */
#define SA_RESETHAND 0x80000000	/* Reset to SIG_DFL on entry to handler.  */

/* Some aliases for the SA_ constants.  */
#define SA_NOMASK	SA_NODEFER
#define SA_ONESHOT	SA_RESETHAND
#endif

/* Values for the HOW argument to `sigprocmask'.  */
#define	SIG_BLOCK	0	/* Block signals.  */
#define	SIG_UNBLOCK	1	/* Unblock signals.  */
#define	SIG_SETMASK	2	/* Set the set of blocked signals.  */
