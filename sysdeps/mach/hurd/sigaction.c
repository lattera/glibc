/* Copyright (C) 1991,92,93,94,95,96,97,2002 Free Software Foundation, Inc.
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

#include <errno.h>
#include <signal.h>
#include <hurd.h>
#include <hurd/signal.h>

/* If ACT is not NULL, change the action for SIG to *ACT.
   If OACT is not NULL, put the old action for SIG in *OACT.  */
int
__sigaction (sig, act, oact)
     int sig;
     const struct sigaction *act;
     struct sigaction *oact;
{
  struct hurd_sigstate *ss;
  struct sigaction a, old;
  sigset_t pending;

  if (sig <= 0 || sig >= NSIG ||
      (act != NULL && act->sa_handler != SIG_DFL &&
       ((__sigmask (sig) & _SIG_CANT_MASK) ||
	act->sa_handler == SIG_ERR)))
    {
      errno = EINVAL;
      return -1;
    }

  /* Copy so we fault before taking locks.  */
  if (act != NULL)
    a = *act;

  ss = _hurd_self_sigstate ();

  __spin_lock (&ss->critical_section_lock);
  __spin_lock (&ss->lock);
  old = ss->actions[sig];
  if (act != NULL)
    ss->actions[sig] = a;

  if (act != NULL && sig == SIGCHLD &&
      (a.sa_flags & SA_NOCLDSTOP) != (old.sa_flags & SA_NOCLDSTOP))
    {
      __spin_unlock (&ss->lock);

      /* Inform the proc server whether or not it should send us SIGCHLD for
	 stopped children.  We do this in a critical section so that no
	 SIGCHLD can arrive in the middle and be of indeterminate status.  */
      __USEPORT (PROC,
		 __proc_mod_stopchild (port, !(a.sa_flags & SA_NOCLDSTOP)));

      __spin_lock (&ss->lock);
      pending = ss->pending & ~ss->blocked;
    }
  else if (a.sa_handler == SIG_IGN || a.sa_handler == SIG_DFL)
    /* We are changing to an action that might be to ignore SIG signals.
       If SIG is blocked and pending and the new action is to ignore it, we
       must remove it from the pending set now; if the action is changed
       back and then SIG is unblocked, the signal pending now should not
       arrive.  So wake up the signal thread to check the new state and do
       the right thing.  */
    pending = ss->pending & __sigmask (sig);
  else
    pending = 0;

  __spin_unlock (&ss->lock);
  __spin_unlock (&ss->critical_section_lock);

  if (pending)
    __msg_sig_post (_hurd_msgport, 0, 0, __mach_task_self ());

  if (oact != NULL)
    *oact = old;

  return 0;
}
libc_hidden_def (__sigaction)
weak_alias (__sigaction, sigaction)
