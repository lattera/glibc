/* Copyright (C) 1991,92,93,94,95,96,97,98 Free Software Foundation, Inc.
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

/*
 *	ISO C Standard: 4.7 SIGNAL HANDLING <signal.h>
 */

#ifndef	_SIGNAL_H

#if !defined __need_sig_atomic_t && !defined __need_sigset_t
# define _SIGNAL_H	1
# include <features.h>
#endif

__BEGIN_DECLS

#include <bits/types.h>
#include <bits/sigset.h>		/* __sigset_t, __sig_atomic_t.  */

#if defined _SIGNAL_H && defined __USE_UNIX98 && !defined pid_t
typedef __pid_t pid_t;
# define pid_t pid_t
#endif	/* Unix98 */

#if !defined __sig_atomic_t_defined \
    && (defined _SIGNAL_H || defined __need_sig_atomic_t)
/* An integral type that can be modified atomically, without the
   possibility of a signal arriving in the middle of the operation.  */
typedef __sig_atomic_t sig_atomic_t;
# define __sig_atomic_t_defined
#endif /* `sig_atomic_t' undefined and <signal.h> or need `sig_atomic_t'.  */
#undef __need_sig_atomic_t

#ifdef _SIGNAL_H

#include <bits/signum.h>

/* Type of a signal handler.  */
typedef void (*__sighandler_t) __P ((int));

/* The X/Open definition of `signal' specifies the SVID semantic.  Use
   the additional function `sysv_signal' when X/Open compatibility is
   requested.  */
extern __sighandler_t __sysv_signal __P ((int __sig,
					  __sighandler_t __handler));

/* Set the handler for the signal SIG to HANDLER, returning the old
   handler, or SIG_ERR on error.
   By default `signal' has the BSD semantic.  */
#ifdef __USE_BSD
extern __sighandler_t signal __P ((int __sig, __sighandler_t __handler));
#else
/* Make sure the used `signal' implementation is the SVID version. */
# ifdef __REDIRECT
extern __sighandler_t __REDIRECT (signal,
				  __P ((int __sig, __sighandler_t __handler)),
				  __sysv_signal);
# else
#  define signal __sysv_signal
# endif
#endif

#ifdef __USE_XOPEN
/* The X/Open definition of `signal' conflicts with the BSD version.
   So they defined another function `bsd_signal'.  */
extern __sighandler_t __bsd_signal __P ((int __sig, __sighandler_t __handler));
extern __sighandler_t bsd_signal __P ((int __sig, __sighandler_t __handler));
#endif

/* Send signal SIG to process number PID.  If PID is zero,
   send SIG to all processes in the current process's process group.
   If PID is < -1, send SIG to all processes in process group - PID.  */
extern int __kill __P ((__pid_t __pid, int __sig));
#ifdef __USE_POSIX
extern int kill __P ((__pid_t __pid, int __sig));
#endif /* Use POSIX.  */

#if defined __USE_BSD || defined __USE_XOPEN_EXTENDED
/* Send SIG to all processes in process group PGRP.
   If PGRP is zero, send SIG to all processes in
   the current process's process group.  */
extern int killpg __P ((__pid_t __pgrp, int __sig));
#endif /* Use BSD || X/Open Unix.  */

/* Raise signal SIG, i.e., send SIG to yourself.  */
extern int raise __P ((int __sig));

#ifdef __USE_SVID
/* SVID names for the same things.  */
extern __sighandler_t ssignal __P ((int __sig, __sighandler_t __handler));
extern int gsignal __P ((int __sig));
#endif /* Use SVID.  */

#ifdef __USE_MISC
/* Print a message describing the meaning of the given signal number.  */
extern void psignal __P ((int __sig, __const char *__s));
#endif /* Use misc.  */


/* Block signals in MASK, returning the old mask.  */
extern int __sigblock __P ((int __mask));

/* Set the mask of blocked signals to MASK, returning the old mask.  */
extern int __sigsetmask __P ((int __mask));


/* The `sigpause' function has two different interfaces.  The original
   BSD definition defines the argument as a mask of the signal, while
   the more modern interface in X/Open defines it as the signal
   number.  We go with the BSD version unless the user explicitly
   selects the X/Open version.  */
extern int __sigpause __P ((int __sig_or_mask, int __is_sig));

#if defined __USE_BSD || defined __USE_GNU
/* Set the mask of blocked signals to MASK,
   wait for a signal to arrive, and then restore the mask.  */
extern int sigpause __P ((int __mask));
# define sigpause(mask) __sigpause ((mask), 0)
#else
# ifdef __USE_XOPEN
/* Remove a signal from the signal mask and suspend the process.  */
#  define sigpause(sig) __sigpause ((sig), 1)
# endif
#endif


#ifdef __USE_BSD
# define sigmask(sig)	__sigmask(sig)

extern int sigblock __P ((int __mask));
extern int sigsetmask __P ((int __mask));

/* This function is here only for compatibility.
   Use `sigprocmask' instead.  */
extern int siggetmask __P ((void));
#endif /* Use BSD.  */


#ifdef __USE_MISC
# define NSIG	_NSIG
#endif

#ifdef __USE_GNU
typedef __sighandler_t sighandler_t;
#endif

/* 4.4 BSD uses the name `sig_t' for this.  */
#ifdef __USE_BSD
typedef __sighandler_t sig_t;
#endif

#endif /* <signal.h> included.  */


# if !defined __sigset_t_defined \
    && ((defined _SIGNAL_H  && defined __USE_POSIX) || defined __need_sigset_t)
typedef __sigset_t sigset_t;
#  define __sigset_t_defined	1
# endif /* `sigset_t' not defined and <signal.h> or need `sigset_t'.  */
# undef __need_sigset_t

#ifdef __USE_POSIX

# ifdef _SIGNAL_H

/* We need `struct timespec' later on.  */
#  define __need_timespec
#  include <time.h>

/* Get the `siginfo_t' type plus the needed symbols.  */
#  include <bits/siginfo.h>

/* Clear all signals from SET.  */
extern int sigemptyset __P ((sigset_t *__set));

/* Set all signals in SET.  */
extern int sigfillset __P ((sigset_t *__set));

/* Add SIGNO to SET.  */
extern int sigaddset __P ((sigset_t *__set, int __signo));

/* Remove SIGNO from SET.  */
extern int sigdelset __P ((sigset_t *__set, int __signo));

/* Return 1 if SIGNO is in SET, 0 if not.  */
extern int sigismember __P ((__const sigset_t *__set, int __signo));

#  ifdef __USE_GNU
/* Return non-empty value is SET is not empty.  */
extern int sigisemptyset __P ((__const sigset_t *__set));

/* Build new signal set by combining the two inputs set using logical AND.  */
extern int sigandset __P ((sigset_t *__set, __const sigset_t *__left,
			   __const sigset_t *__right));

/* Build new signal set by combining the two inputs set using logical OR.  */
extern int sigorset __P ((sigset_t *__set, __const sigset_t *__left,
			  __const sigset_t *__right));
#  endif /* GNU */

/* Get the system-specific definitions of `struct sigaction'
   and the `SA_*' and `SIG_*'. constants.  */
#  include <bits/sigaction.h>

/* Get and/or change the set of blocked signals.  */
extern int __sigprocmask __P ((int __how,
			       __const sigset_t *__set, sigset_t *__oset));
extern int sigprocmask __P ((int __how,
			     __const sigset_t *__set, sigset_t *__oset));

/* Change the set of blocked signals to SET,
   wait until a signal arrives, and restore the set of blocked signals.  */
extern int __sigsuspend __P ((__const sigset_t *__set));
extern int sigsuspend __P ((__const sigset_t *__set));

/* Get and/or set the action for signal SIG.  */
extern int __sigaction __P ((int __sig, __const struct sigaction *__act,
			     struct sigaction *__oact));
extern int sigaction __P ((int __sig, __const struct sigaction *__act,
			   struct sigaction *__oact));

/* Put in SET all signals that are blocked and waiting to be delivered.  */
extern int sigpending __P ((sigset_t *__set));


/* Select any of pending signals from SET or wait for any to arrive.  */
extern int __sigwait __P ((__const sigset_t *__set, int *__sig));
extern int sigwait __P ((__const sigset_t *__set, int *__sig));

/* Select any of pending signals from SET and place information in INFO.  */
extern int __sigwaitinfo __P ((__const sigset_t *__set, siginfo_t *__info));
extern int sigwaitinfo __P ((__const sigset_t *__set, siginfo_t *__info));

/* Select any of pending signals from SET and place information in INFO.
   Wait the imte specified by TIMEOUT if no signal is pending.  */
extern int __sigtimedwait __P ((__const sigset_t *__set, siginfo_t *__info,
				__const struct timespec *__timeout));
extern int sigtimedwait __P ((__const sigset_t *__set, siginfo_t *__info,
			      __const struct timespec *__timeout));

/* Send signal SIG to the process PID.  Associate data in VAL with the
   signal.  */
extern int __sigqueue __P ((__pid_t __pid, int __sig,
			    __const union sigval __val));
extern int sigqueue __P ((__pid_t __pid, int __sig,
			  __const union sigval __val));

# endif /* <signal.h> included.  */

#endif /* Use POSIX.  */

#if defined _SIGNAL_H && defined __USE_BSD

/* Names of the signals.  This variable exists only for compatibility.
   Use `strsignal' instead (see <string.h>).  */
extern __const char *__const _sys_siglist[_NSIG];
extern __const char *__const sys_siglist[_NSIG];

/* Structure passed to `sigvec'.  */
struct sigvec
  {
    __sighandler_t sv_handler;	/* Signal handler.  */
    int sv_mask;		/* Mask of signals to be blocked.  */

    int sv_flags;		/* Flags (see below).  */
# define sv_onstack	sv_flags /* 4.2 BSD compatibility.  */
  };

/* Bits in `sv_flags'.  */
# define SV_ONSTACK	(1 << 0)/* Take the signal on the signal stack.  */
# define SV_INTERRUPT	(1 << 1)/* Do not restart system calls.  */
# define SV_RESETHAND	(1 << 2)/* Reset handler to SIG_DFL on receipt.  */


/* If VEC is non-NULL, set the handler for SIG to the `sv_handler' member
   of VEC.  The signals in `sv_mask' will be blocked while the handler runs.
   If the SV_RESETHAND bit is set in `sv_flags', the handler for SIG will be
   reset to SIG_DFL before `sv_handler' is entered.  If OVEC is non-NULL,
   it is filled in with the old information for SIG.  */
extern int __sigvec __P ((int __sig, __const struct sigvec *__vec,
			  struct sigvec *__ovec));
extern int sigvec __P ((int __sig, __const struct sigvec *__vec,
			struct sigvec *__ovec));


/* Get machine-dependent `struct sigcontext' and signal subcodes.  */
# include <bits/sigcontext.h>

/* Restore the state saved in SCP.  */
extern int __sigreturn __P ((struct sigcontext *__scp));
extern int sigreturn __P ((struct sigcontext *__scp));

#endif /* signal.h included and use BSD.  */


#if defined _SIGNAL_H && (defined __USE_BSD || defined __USE_XOPEN_EXTENDED)

# define __need_size_t
# include <stddef.h>

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
typedef struct sigaltstack
  {
    __ptr_t ss_sp;
    int ss_flags;
    size_t ss_size;
  } stack_t;

extern int sigaltstack __P ((__const struct sigaltstack *__ss,
			     struct sigaltstack *__oss));

#endif /* signal.h included and use BSD or X/Open Unix.  */


#if defined _SIGNAL_H && defined __USE_UNIX98
/* Simplified interface for signal management.  */

/* Add SIG to the calling process' signal mask.  */
extern int sighold __P ((int __sig));

/* Remove SIG from the calling process' signal mask.  */
extern int sigrelse __P ((int __sig));

/* Set the disposition of SIG to SIG_IGN.  */
extern int sigignore __P ((int __sig));

/* Set the disposition of SIG.  */
extern __sighandler_t sigset __P ((int __sig, __sighandler_t __disp));
#endif


/* The following functions are used internally in the C library and in
   other code which need deep insights.  */

/* Return number of available real-time signal with highest priority.  */
extern int __libc_current_sigrtmin __P ((void));
/* Return number of available real-time signal with lowest priority.  */
extern int __libc_current_sigrtmax __P ((void));

/* Allocate real-time signal with highest/lowest available priority.  */
extern int __libc_allocate_rtsig __P ((int __high));

__END_DECLS

#endif /* signal.h  */
