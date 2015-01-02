/* Copyright (C) 1991-2015 Free Software Foundation, Inc.

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

#include <errno.h>
#include <hurd.h>
#include <hurd/signal.h>
#include <hurd/msg.h>

/* Change the set of blocked signals to SET,
   wait until a signal arrives, and restore the set of blocked signals.  */
int
__sigsuspend (set)
     const sigset_t *set;
{
  struct hurd_sigstate *ss;
  sigset_t newmask, oldmask, pending;
  mach_port_t wait;
  mach_msg_header_t msg;

  if (set != NULL)
    /* Crash before locking.  */
    newmask = *set;

  /* Get a fresh port we will wait on.  */
  wait = __mach_reply_port ();

  ss = _hurd_self_sigstate ();

  __spin_lock (&ss->lock);

  oldmask = ss->blocked;
  if (set != NULL)
    /* Change to the new blocked signal mask.  */
    ss->blocked = newmask & ~_SIG_CANT_MASK;

  /* Notice if any pending signals just became unblocked.  */
  pending = ss->pending & ~ss->blocked;

  /* Tell the signal thread to message us when a signal arrives.  */
  ss->suspended = wait;
  __spin_unlock (&ss->lock);

  if (pending)
    /* Tell the signal thread to check for pending signals.  */
    __msg_sig_post (_hurd_msgport, 0, 0, __mach_task_self ());

  /* Wait for the signal thread's message.  */
  __mach_msg (&msg, MACH_RCV_MSG, 0, sizeof (msg), wait,
	      MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
  __mach_port_destroy (__mach_task_self (), wait);

  __spin_lock (&ss->lock);
  ss->blocked = oldmask;	/* Restore the old mask.  */
  pending = ss->pending & ~ss->blocked;	/* Again check for pending signals.  */
  __spin_unlock (&ss->lock);

  if (pending)
    /* Tell the signal thread to check for pending signals.  */
    __msg_sig_post (_hurd_msgport, 0, 0, __mach_task_self ());

  /* We've been interrupted!  And a good thing, too.
     Otherwise we'd never return.
     That's right; this function always returns an error.  */
  errno = EINTR;
  return -1;
}
libc_hidden_def (__sigsuspend)
strong_alias (__sigsuspend, sigsuspend_not_cancel)
weak_alias (__sigsuspend, sigsuspend)
