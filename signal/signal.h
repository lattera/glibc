/* Copyright (C) 1991, 1992, 1993, 1994 Free Software Foundation, Inc.
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
not, write to the, 1992 Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

/*
 *	ANSI Standard: 4.7 SIGNAL HANDLING <signal.h>
 */

#ifndef	_SIGNAL_H

#if	!defined(__need_sig_atomic_t) && !defined(__need_sigset_t)
#define	_SIGNAL_H	1
#include <features.h>
#endif

__BEGIN_DECLS

#define	 __need_size_t
#include <stddef.h>

#include <gnu/types.h>
#include <sigset.h>		/* __sigset_t, __sig_atomic_t.  */

#if	!defined(__sig_atomic_t_defined) &&	\
  (defined(_SIGNAL_H) || defined(__need_sig_atomic_t))
/* An integral type that can be modified atomically, without the
   possibility of a signal arriving in the middle of the operation.  */
typedef __sig_atomic_t sig_atomic_t;
#endif /* `sig_atomic_t' undefined and <signal.h> or need `sig_atomic_t'.  */
#undef	__need_sig_atomic_t

#ifdef	_SIGNAL_H

#include <signum.h>

/* Type of a signal handler.  */
typedef void (*__sighandler_t) __P ((int));

/* Set the handler for the signal SIG to HANDLER,
   returning the old handler, or SIG_ERR on error.  */
extern __sighandler_t signal __P ((int __sig, __sighandler_t __handler));

/* Send signal SIG to process number PID.  If PID is zero,
   send SIG to all processes in the current process's process group.
   If PID is < -1, send SIG to all processes in process group - PID.  */
extern int __kill __P ((__pid_t __pid, int __sig));
#ifdef	__USE_POSIX
extern int kill __P ((__pid_t __pid, int __sig));
#endif /* Use POSIX.  */

#ifdef	__USE_BSD
/* Send SIG to all processes in process group PGRP.
   If PGRP is zero, send SIG to all processes in
   the current process's process group.  */
extern int killpg __P ((__pid_t __pgrp, int __sig));
#endif /* Use BSD.  */

/* Raise signal SIG, i.e., send SIG to yourself.  */
extern int raise __P ((int __sig));

#ifdef	__USE_SVID
/* SVID names for the same things.  */
extern __sighandler_t ssignal __P ((int __sig, __sighandler_t __handler));
extern int gsignal __P ((int __sig));
#endif /* Use SVID.  */

#ifdef	__USE_MISC
/* Print a message describing the meaning of the given signal number.  */
extern void psignal __P ((int __sig, __const char *__s));
#endif /* Use misc.  */


/* Block signals in MASK, returning the old mask.  */
extern int __sigblock __P ((int __mask));

/* Set the mask of blocked signals to MASK, returning the old mask.  */
extern int __sigsetmask __P ((int __mask));

/* Set the mask of blocked signals to MASK,
   wait for a signal to arrive, and then restore the mask.  */
extern int __sigpause __P ((int __mask));

#ifdef	__USE_BSD
#define	sigmask(sig)	__sigmask(sig)

extern int sigblock __P ((int __mask));
extern int sigsetmask __P ((int __mask));
extern int sigpause __P ((int __mask));
#endif /* Use BSD.  */


#ifdef	__USE_MISC
#define	NSIG	_NSIG
#endif

#ifdef	__USE_GNU
typedef __sighandler_t sighandler_t;
#endif

#endif /* <signal.h> included.  */


#ifdef	__USE_POSIX

#if	!defined(__sigset_t_defined) &&	\
   (defined(_SIGNAL_H) || defined(__need_sigset_t))
typedef __sigset_t sigset_t;
#define	__sigset_t_defined	1
#endif /* `sigset_t' not defined and <signal.h> or need `sigset_t'.  */
#undef	__need_sigset_t

#ifdef	_SIGNAL_H

/* Clear all signals from SET.  */
extern int sigemptyset __P ((sigset_t *__set));

/* Set all signals in SET.  */
extern int sigfillset __P ((sigset_t *__set));

/* Add SIGNO to SET.  */
extern int sigaddset __P ((sigset_t *__set, int __signo));

/* Remove SIGNO from SET.  */
extern int sigdelset __P ((sigset_t *__set, int __signo));

/* Return 1 if SIGNO is in SET, 0 if not.  */
extern int sigismember __P ((__const sigset_t *__set, int signo));

#ifdef	__OPTIMIZE__
/* <sigset.h> defines the __ versions as macros that do the work.  */
#define	sigemptyset(set)	__sigemptyset(set)
#define	sigfillset(set)		__sigfillset(set)
#define	sigaddset(set, signo)	__sigaddset(set, signo)
#define	sigdelset(set, signo)	__sigdelset(set, signo)
#define	sigismember(set, signo)	__sigismember(set, signo)
#endif

/* Get the system-specific definitions of `struct sigaction'
   and the `SA_*' and `SIG_*'. constants.  */
#include <sigaction.h>

/* Get and/or change the set of blocked signals.  */
extern int __sigprocmask __P ((int __how,
			       __const sigset_t *__set, sigset_t *__oset));
extern int sigprocmask __P ((int __how,
			     __const sigset_t *__set, sigset_t *__oset));

/* Change the set of blocked signals to SET,
   wait until a signal arrives, and restore the set of blocked signals.  */
extern int sigsuspend __P ((__const sigset_t *__set));

/* Get and/or set the action for signal SIG.  */
extern int __sigaction __P ((int __sig, __const struct sigaction *__act,
			     struct sigaction *__oact));
extern int sigaction __P ((int __sig, __const struct sigaction *__act,
			   struct sigaction *__oact));

/* Put in SET all signals that are blocked and waiting to be delivered.  */
extern int sigpending __P ((sigset_t *__set));

#endif /* <signal.h> included.  */

#endif /* Use POSIX.  */

#if	defined(_SIGNAL_H) && defined(__USE_BSD)

/* Structure passed to `sigvec'.  */
struct sigvec
  {
    __sighandler_t sv_handler;	/* Signal handler.  */
    int sv_mask;		/* Mask of signals to be blocked.  */

    int sv_flags;		/* Flags (see below).  */
#define	sv_onstack	sv_flags /* 4.2 BSD compatibility.  */
  };

/* Bits in `sv_flags'.  */
#define	SV_ONSTACK	(1 << 0)/* Take the signal on the signal stack.  */
#define	SV_INTERRUPT	(1 << 1)/* Do not restart system calls.  */
#define	SV_RESETHAND	(1 << 2)/* Reset handler to SIG_DFL on receipt.  */


/* If VEC is non-NULL, set the handler for SIG to the `sv_handler' member
   of VEC.  The signals in `sv_mask' will be blocked while the handler runs.
   If the SV_RESETHAND bit is set in `sv_flags', the handler for SIG will be
   reset to SIG_DFL before `sv_handler' is entered.  If OVEC is non-NULL,
   it is filled in with the old information for SIG.  */
extern int __sigvec __P ((int __sig, __const struct sigvec *__vec,
			  struct sigvec *__ovec));
extern int sigvec __P ((int __sig, __const struct sigvec *__vec,
			struct sigvec *__ovec));


/* If INTERRUPT is nonzero, make signal SIG interrupt system calls
   (causing them to fail with EINTR); if INTERRUPT is zero, make system
   calls be restarted after signal SIG.  */
extern int siginterrupt __P ((int __sig, int __interrupt));


/* Structure describing a signal stack.  */
struct sigstack
  {
    __ptr_t ss_sp;		/* Signal stack pointer.  */
    int ss_onstack;		/* Nonzero if executing on this stack.  */
  };

/* Run signals handlers on the stack specified by SS (if not NULL).
   If OSS is not NULL, it is filled in with the old signal stack status.  */
extern int sigstack __P ((__const struct sigstack *__ss,
			  struct sigstack *__oss));

/* Alternate interface.  */
struct sigaltstack
  {
    __ptr_t ss_sp;
    size_t ss_size;
    int ss_flags;
  };

extern int sigaltstack __P ((__const struct sigaltstack *__ss,
			     struct sigaltstack *__oss));

/* Get machine-dependent `struct sigcontext' and signal subcodes.  */
#include <sigcontext.h>

/* Restore the state saved in SCP.  */
extern int __sigreturn __P ((struct sigcontext *__scp));
extern int sigreturn __P ((struct sigcontext *__scp));

#endif /* signal.h included and use BSD.  */

__END_DECLS

#endif /* signal.h  */
