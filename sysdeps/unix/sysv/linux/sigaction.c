/* Copyright (C) 1997, 1998 Free Software Foundation, Inc.
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

#include <errno.h>
#include <signal.h>
#include <string.h>

#include <sysdep.h>
#include <sys/syscall.h>

/* The difference here is that the sigaction structure used in the
   kernel is not the same as we use in the libc.  Therefore we must
   translate it here.  */
#include <kernel_sigaction.h>

extern int __syscall_sigaction (int, const struct old_kernel_sigaction *,
				struct old_kernel_sigaction *);
extern int __syscall_rt_sigaction (int, const struct kernel_sigaction *,
				   struct kernel_sigaction *, size_t);

/* The variable is shared between all wrappers around signal handling
   functions which have RT equivalents.  */
int __libc_missing_rt_sigs;


/* If ACT is not NULL, change the action for SIG to *ACT.
   If OACT is not NULL, put the old action for SIG in *OACT.  */
int
__sigaction (sig, act, oact)
     int sig;
     const struct sigaction *act;
     struct sigaction *oact;
{
  struct old_kernel_sigaction k_sigact, k_osigact;
  int result;

  /* First try the RT signals.  */
  if (!__libc_missing_rt_sigs)
    {
      struct kernel_sigaction kact, koact;
      int saved_errno = errno;

      if (act)
	{
	  kact.k_sa_handler = act->sa_handler;
	  memcpy (&kact.sa_mask, &act->sa_mask, sizeof (sigset_t));
	  kact.sa_flags = act->sa_flags;
#ifdef HAVE_SA_RESTORER
	  kact.sa_restorer = act->sa_restorer;
#endif
	}

      /* XXX The size argument hopefully will have to be changed to the
	 real size of the user-level sigset_t.  */
      result = INLINE_SYSCALL (rt_sigaction, 4, sig, act ? &kact : 0,
			       oact ? &koact : 0, _NSIG / 8);

      if (result >= 0 || errno != ENOSYS)
	{
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

      __set_errno (saved_errno);
      __libc_missing_rt_sigs = 1;
    }

  if (act)
    {
      k_sigact.k_sa_handler = act->sa_handler;
      k_sigact.sa_mask = act->sa_mask.__val[0];
      k_sigact.sa_flags = act->sa_flags;
#ifdef HAVE_SA_RESTORER
      k_sigact.sa_restorer = act->sa_restorer;
#endif
    }
  result = INLINE_SYSCALL (sigaction, 3, sig, act ? &k_sigact : 0,
			   oact ? &k_osigact : 0);
  if (oact && result >= 0)
    {
      oact->sa_handler = k_osigact.k_sa_handler;
      oact->sa_mask.__val[0] = k_osigact.sa_mask;
      oact->sa_flags = k_osigact.sa_flags;
#ifdef HAVE_SA_RESTORER
      oact->sa_restorer = k_osigact.sa_restorer;
#endif
    }
  return result;
}

weak_alias (__sigaction, sigaction)
