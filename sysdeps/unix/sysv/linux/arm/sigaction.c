/* Copyright (C) 1997, 1998, 1999, 2000, 2002, 2003, 2005, 2006
   Free Software Foundation, Inc.
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
#include <string.h>

#include <sysdep.h>
#include <sys/syscall.h>
#include <kernel-features.h>

/* The difference here is that the sigaction structure used in the
   kernel is not the same as we use in the libc.  Therefore we must
   translate it here.  */
#include <kernel_sigaction.h>

/* The variable is shared between all wrappers around signal handling
   functions which have RT equivalents.  */
int __libc_missing_rt_sigs;

#define SA_RESTORER	0x04000000

#ifdef __ARM_EABI__
extern void __default_sa_restorer_v1(void);
extern void __default_sa_restorer_v2(void);
extern void __default_rt_sa_restorer_v1(void);
extern void __default_rt_sa_restorer_v2(void);
# ifdef __ASSUME_SIGFRAME_V2
#  define __default_sa_restorer __default_sa_restorer_v2
#  define __default_rt_sa_restorer __default_rt_sa_restorer_v2
# else
#  include <ldsodefs.h>
#  define __default_sa_restorer (GLRO(dl_osversion) >= 0x020612	\
				 ? __default_sa_restorer_v2	\
				 : __default_sa_restorer_v1)
#  define __default_rt_sa_restorer (GLRO(dl_osversion) >= 0x020612	\
				    ? __default_rt_sa_restorer_v2	\
				    : __default_rt_sa_restorer_v1)
# endif
#else
extern void __default_sa_restorer(void);
extern void __default_rt_sa_restorer(void);
#endif

/* When RT signals are in use we need to use a different return stub.  */
#ifdef __NR_rt_sigreturn
#define choose_restorer(flags)					\
  (flags & SA_SIGINFO) ? __default_rt_sa_restorer		\
  : __default_sa_restorer
#else
#define choose_restorer(flags)					\
  __default_sa_restorer
#endif

/* If ACT is not NULL, change the action for SIG to *ACT.
   If OACT is not NULL, put the old action for SIG in *OACT.  */
int
__libc_sigaction (sig, act, oact)
     int sig;
     const struct sigaction *act;
     struct sigaction *oact;
{
#ifndef __ASSUME_REALTIME_SIGNALS
  struct old_kernel_sigaction k_sigact, k_osigact;
#endif
  int result;

#ifdef __NR_rt_sigaction
  /* First try the RT signals.  */
# ifndef __ASSUME_REALTIME_SIGNALS
  if (!__libc_missing_rt_sigs)
# endif
    {
      struct kernel_sigaction kact, koact;
# ifndef __ASSUME_REALTIME_SIGNALS
      int saved_errno = errno;
# endif

      if (act)
	{
	  kact.k_sa_handler = act->sa_handler;
	  memcpy (&kact.sa_mask, &act->sa_mask, sizeof (sigset_t));
	  kact.sa_flags = act->sa_flags;
# ifdef HAVE_SA_RESTORER
	  if (kact.sa_flags & SA_RESTORER)
	    kact.sa_restorer = act->sa_restorer;
	  else
	    {
	      kact.sa_restorer = choose_restorer (kact.sa_flags);
	      kact.sa_flags |= SA_RESTORER;
	    }
# endif
	}

      /* XXX The size argument hopefully will have to be changed to the
	 real size of the user-level sigset_t.  */
      result = INLINE_SYSCALL (rt_sigaction, 4, sig,
			       act ? __ptrvalue (&kact) : NULL,
			       oact ? __ptrvalue (&koact) : NULL, _NSIG / 8);

# ifndef __ASSUME_REALTIME_SIGNALS
      if (result >= 0 || errno != ENOSYS)
# endif
	{
	  if (oact && result >= 0)
	    {
	      oact->sa_handler = koact.k_sa_handler;
	      memcpy (&oact->sa_mask, &koact.sa_mask, sizeof (sigset_t));
	      oact->sa_flags = koact.sa_flags;
# ifdef HAVE_SA_RESTORER
	      oact->sa_restorer = koact.sa_restorer;
# endif
	    }
	  return result;
	}

# ifndef __ASSUME_REALTIME_SIGNALS
      __set_errno (saved_errno);
      __libc_missing_rt_sigs = 1;
# endif
    }
#endif

#ifndef __ASSUME_REALTIME_SIGNALS
  if (act)
    {
      k_sigact.k_sa_handler = act->sa_handler;
      k_sigact.sa_mask = act->sa_mask.__val[0];
      k_sigact.sa_flags = act->sa_flags;
# ifdef HAVE_SA_RESTORER
      if (k_sigact.sa_flags & SA_RESTORER)
	k_sigact.sa_restorer = act->sa_restorer;
      else
	{
	  k_sigact.sa_restorer = choose_restorer (k_sigact.sa_flags);
	  k_sigact.sa_flags |= SA_RESTORER;
	}
# endif
    }
  result = INLINE_SYSCALL (sigaction, 3, sig,
			   act ? __ptrvalue (&k_sigact) : NULL,
			   oact ? __ptrvalue (&k_osigact) : NULL);
  if (oact && result >= 0)
    {
      oact->sa_handler = k_osigact.k_sa_handler;
      oact->sa_mask.__val[0] = k_osigact.sa_mask;
      oact->sa_flags = k_osigact.sa_flags;
# ifdef HAVE_SA_RESTORER
      oact->sa_restorer = k_osigact.sa_restorer;
# endif
    }
  return result;
#endif
}
libc_hidden_def (__libc_sigaction)

#ifdef WRAPPER_INCLUDE
# include WRAPPER_INCLUDE
#endif

#ifndef LIBC_SIGACTION
weak_alias (__libc_sigaction, __sigaction)
libc_hidden_weak (__sigaction)
weak_alias (__libc_sigaction, sigaction)
#endif
