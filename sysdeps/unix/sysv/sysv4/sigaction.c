/* Copyright (C) 1994, 1995, 1996, 1997 Free Software Foundation, Inc.
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
#include <stddef.h>

static __sighandler_t user_handlers[NSIG];

extern int __context_syscall (int, struct sigcontext *);
extern int __sigaction_syscall (int,
				const struct sigaction *, struct sigaction *);

static void
trampoline (int sig, int code, struct sigcontext *context)
{
  (*(void (*) (int, int, struct sigcontext *)) user_handlers[sig])
    (sig, code, context);
  __context_syscall (1, context);
}

/* If ACT is not NULL, change the action for SIG to *ACT.
   If OACT is not NULL, put the old action for SIG in *OACT.  */
int
__sigaction (sig, act, oact)
     int sig;
     const struct sigaction *act;
     struct sigaction *oact;
{
  struct sigaction myact;
  __sighandler_t ohandler;

  if (sig <= 0 || sig >= NSIG)
    {
      __set_errno (EINVAL);
      return -1;
    }

  ohandler = user_handlers[sig];

  if (act != NULL)
    {
      user_handlers[sig] = act->sa_handler;
      if (act->sa_handler != SIG_DFL && act->sa_handler != SIG_IGN)
	{
	  myact = *act;
	  act = &myact;
	  act->sa_handler = (__sighandler_t) trampoline;
	}
    }

  if (__sigaction_syscall (sig, act, oact) < 0)
    {
      /* The syscall got an error.  Restore the old handler and return -1.  */
      user_handlers[sig] = ohandler;
      return -1;
    }

  if (oact != NULL && oact->sa_handler == (__sighandler_t) trampoline)
    oact->sa_handler = ohandler;

  return 0;
}

weak_alias (__sigaction, sigaction)
