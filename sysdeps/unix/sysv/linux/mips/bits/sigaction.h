/* The proper definitions for Linux/MIPS's sigaction.
   Copyright (C) 1993, 1994, 1995, 1997, 1998 Free Software Foundation, Inc.
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
    /* Special flags.  */
    unsigned int sa_flags;

    /* Signal handler.  */
#ifdef __USE_POSIX199309
    union
      {
	/* Used if SA_SIGINFO is not set.  */
	__sighandler_t sa_handler;
	/* Used if SA_SIGINFO is set.  */
	void (*sa_sigaction) __PMT ((int, siginfo_t *, void *));
      }
    __sigaction_handler;
# define sa_handler    __sigaction_handler.sa_handler
# define sa_sigaction  __sigaction_handler.sa_sigaction
#else
    __sighandler_t sa_handler;
#endif
    /* Additional set of signals to be blocked.  */
    __sigset_t sa_mask;

    /* The ABI says here are two unused ints following. */
    /* Restore handler.  */
    void (*sa_restorer) __PMT ((void));

#if _MIPS_ISA == _MIPS_ISA_MIPS1 || _MIPS_ISA == _MIPS_ISA_MIPS2
    int sa_resv[1];
#endif
  };

/* Bits in `sa_flags'.  */
#define SA_NOCLDSTOP  0x00020000 /* Don't send SIGCHLD when children stop.  */
#define SA_SIGINFO    0x00000008 /* Invoke signal-catching function with
				    three arguments instead of one.  */
#if defined __USE_UNIX98 || defined __USE_MISC
# define SA_ONSTACK   0x00000001 /* Use signal stack by using `sa_restorer'. */
# define SA_RESTART   0x00000004 /* Restart syscall on signal return.  */
# define SA_NODEFER   0x00000010 /* Don't automatically block the signal when
				    its handler is being executed.  */
#endif
#ifdef __USE_MISC
# define SA_INTERRUPT 0x01000000 /* Historical no-op.  */

/* Some aliases for the SA_ constants.  */
# define SA_NOMASK    SA_NODEFER
# define SA_ONESHOT   SA_RESETHAND
# define SA_STACK     SA_ONSTACK
#endif

/* Values for the HOW argument to `sigprocmask'.  */
#define SIG_NOP	      0		/* 0 is unused to catch errors */
#define	SIG_BLOCK     1		/* Block signals.  */
#define	SIG_UNBLOCK   2		/* Unblock signals.  */
#define	SIG_SETMASK   3		/* Set the set of blocked signals.  */
#ifdef __USE_MISC
# define SIG_SETMASK32 256	/* Goodie from SGI for BSD compatibility:
				   set only the low 32 bit of the sigset.  */
#endif
