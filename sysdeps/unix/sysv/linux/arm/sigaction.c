/* Copyright (C) 1997-2015 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <signal.h>
#include <string.h>

#include <sysdep.h>
#include <sys/syscall.h>

/* The difference here is that the sigaction structure used in the
   kernel is not the same as we use in the libc.  Therefore we must
   translate it here.  */
#include <kernel_sigaction.h>

#define SA_RESTORER	0x04000000

extern void __default_sa_restorer (void);
extern void __default_rt_sa_restorer (void);

/* When RT signals are in use we need to use a different return stub.  */
#define choose_restorer(flags)					\
  (flags & SA_SIGINFO) ? __default_rt_sa_restorer		\
  : __default_sa_restorer

/* If ACT is not NULL, change the action for SIG to *ACT.
   If OACT is not NULL, put the old action for SIG in *OACT.  */
int
__libc_sigaction (sig, act, oact)
     int sig;
     const struct sigaction *act;
     struct sigaction *oact;
{
  int result;

  struct kernel_sigaction kact, koact;

  if (act)
    {
      kact.k_sa_handler = act->sa_handler;
      memcpy (&kact.sa_mask, &act->sa_mask, sizeof (sigset_t));
      kact.sa_flags = act->sa_flags;
#ifdef HAVE_SA_RESTORER
      if (kact.sa_flags & SA_RESTORER)
	kact.sa_restorer = act->sa_restorer;
      else
	{
	  kact.sa_restorer = choose_restorer (kact.sa_flags);
	  kact.sa_flags |= SA_RESTORER;
	}
#endif
    }

  /* XXX The size argument hopefully will have to be changed to the
     real size of the user-level sigset_t.  */
  result = INLINE_SYSCALL (rt_sigaction, 4, sig,
			   act ? &kact : NULL,
			   oact ? &koact : NULL, _NSIG / 8);

  if (oact && result >= 0)
    {
      oact->sa_handler = koact.k_sa_handler;
      memcpy (&oact->sa_mask, &koact.sa_mask, sizeof (sigset_t));
      oact->sa_flags = koact.sa_flags;
#ifdef HAVE_SA_RESTORER
      oact->sa_restorer = koact.sa_restorer;
#endif
    }
  return result;
}
libc_hidden_def (__libc_sigaction)

#include <nptl/sigaction.c>
