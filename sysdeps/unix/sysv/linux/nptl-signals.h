/* Special use of signals in NPTL internals.  Linux version.
   Copyright (C) 2014-2016 Free Software Foundation, Inc.
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

#include <signal.h>

/* The signal used for asynchronous cancelation.  */
#define SIGCANCEL       __SIGRTMIN


/* Signal needed for the kernel-supported POSIX timer implementation.
   We can reuse the cancellation signal since we can distinguish
   cancellation from timer expirations.  */
#define SIGTIMER        SIGCANCEL


/* Signal used to implement the setuid et.al. functions.  */
#define SIGSETXID       (__SIGRTMIN + 1)


/* Return is sig is used internally.  */
static inline int
__nptl_is_internal_signal (int sig)
{
  return (sig == SIGCANCEL) || (sig == SIGTIMER) || (sig == SIGSETXID);
}

/* Remove internal glibc signal from the mask.  */
static inline void
__nptl_clear_internal_signals (sigset_t *set)
{
  __sigdelset (set, SIGCANCEL);
  __sigdelset (set, SIGTIMER);
  __sigdelset (set, SIGSETXID);
}

#define SIGALL_SET \
  ((__sigset_t) { .__val = {[0 ...  _SIGSET_NWORDS-1 ] =  -1 } })

/* Block all signals, including internal glibc ones.  */
static inline int
__libc_signal_block_all (sigset_t *set)
{
  INTERNAL_SYSCALL_DECL (err);
  return INTERNAL_SYSCALL (rt_sigprocmask, err, 4, SIG_BLOCK, &SIGALL_SET,
			   set, _NSIG / 8);
}

/* Block all application signals (excluding internal glibc ones).  */
static inline int
__libc_signal_block_app (sigset_t *set)
{
  sigset_t allset = SIGALL_SET;
  __nptl_clear_internal_signals (&allset);
  INTERNAL_SYSCALL_DECL (err);
  return INTERNAL_SYSCALL (rt_sigprocmask, err, 4, SIG_BLOCK, &allset, set,
			   _NSIG / 8);
}

/* Restore current process signal mask.  */
static inline int
__libc_signal_restore_set (const sigset_t *set)
{
  INTERNAL_SYSCALL_DECL (err);
  return INTERNAL_SYSCALL (rt_sigprocmask, err, 4, SIG_SETMASK, set, NULL,
			   _NSIG / 8);
}

/* Used to communicate with signal handler.  */
extern struct xid_command *__xidcmd attribute_hidden;
