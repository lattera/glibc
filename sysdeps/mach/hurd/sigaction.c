/* Copyright (C) 1991, 1992, 1993, 1994, 1995 Free Software Foundation, Inc.
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
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <ansidecl.h>
#include <errno.h>
#include <signal.h>
#include <hurd.h>
#include <hurd/signal.h>

/* If ACT is not NULL, change the action for SIG to *ACT.
   If OACT is not NULL, put the old action for SIG in *OACT.  */
int
DEFUN(__sigaction, (sig, act, oact),
      int sig AND CONST struct sigaction *act AND struct sigaction *oact)
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

  __spin_lock (&ss->lock);
  old = ss->actions[sig];
  if (act != NULL)
    ss->actions[sig] = a;

  if (act != NULL && sig == SIGCHLD)
    {
      ss->critical_section = 1;
      __spin_unlock (&ss->lock);

      /* Inform the proc server whether or not it should send us SIGCHLD for
	 stopped children.  We do this in a critical section so that no
	 SIGCHLD can arrive in the middle and be of indeterminate status.  */
      __USEPORT (PROC,
		 __proc_mod_stopchild (port, !(a.sa_flags & SA_NOCLDSTOP)));

      __spin_lock (&ss->lock);
      ss->critical_section = 0;
      pending = ss->pending & ~ss->blocked;
    }
  else
    pending = 0;

  __spin_unlock (&ss->lock);

  if (pending)
    __msg_sig_post (_hurd_msgport, 0, __mach_task_self ());

  if (oact != NULL)
    *oact = old;

  return 0;
}

weak_alias (__sigaction, sigaction)
