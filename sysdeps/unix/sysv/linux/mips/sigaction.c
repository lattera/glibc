/* Copyright (C) 1997,1998,1999,2000,2002,2003,2004,2006
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
#include <sgidefs.h>
#include <signal.h>
#include <string.h>

#include <sysdep.h>
#include <sys/syscall.h>

#include <sgidefs.h>

#include <kernel-features.h>

/* The difference here is that the sigaction structure used in the
   kernel is not the same as we use in the libc.  Therefore we must
   translate it here.  */
#include <kernel_sigaction.h>

#if __ASSUME_REALTIME_SIGNALS == 0
/* The variable is shared between all wrappers around signal handling
   functions which have RT equivalents.	 This is the definition.  */
int __libc_missing_rt_sigs;

#endif

#if _MIPS_SIM != _ABIO32

# ifdef __NR_rt_sigreturn
static void restore_rt (void) asm ("__restore_rt");
# endif
# ifdef __NR_sigreturn
static void restore (void) asm ("__restore");
# endif
#endif

/* If ACT is not NULL, change the action for SIG to *ACT.
   If OACT is not NULL, put the old action for SIG in *OACT.  */
int
__libc_sigaction (sig, act, oact)
     int sig;
     const struct sigaction *act;
     struct sigaction *oact;
{
#if __ASSUME_REALTIME_SIGNALS == 0
  struct old_kernel_sigaction k_sigact, k_osigact;
#endif
  int result;

#if defined __NR_rt_sigaction || __ASSUME_REALTIME_SIGNALS > 0
  /* First try the RT signals.	*/
# if __ASSUME_REALTIME_SIGNALS == 0
  if (!__libc_missing_rt_sigs)
# endif
    {
      struct kernel_sigaction kact, koact;
      /* Save the current error value for later.  We need not do this
	 if we are guaranteed to have realtime signals.	 */
# if __ASSUME_REALTIME_SIGNALS == 0
      int saved_errno = errno;
# endif

      if (act)
	{
	  kact.k_sa_handler = act->sa_handler;
	  memcpy (&kact.sa_mask, &act->sa_mask, sizeof (kernel_sigset_t));
	  kact.sa_flags = act->sa_flags;
# ifdef HAVE_SA_RESTORER
#  if _MIPS_SIM == _ABIO32
	  kact.sa_restorer = act->sa_restorer;
#  else
	  kact.sa_restorer = &restore_rt;
#  endif
# endif
	}

      /* XXX The size argument hopefully will have to be changed to the
	 real size of the user-level sigset_t.	*/
      result = INLINE_SYSCALL (rt_sigaction, 4, sig,
			       act ? __ptrvalue (&kact) : NULL,
			       oact ? __ptrvalue (&koact) : NULL,
			       sizeof (kernel_sigset_t));

# if __ASSUME_REALTIME_SIGNALS == 0
      if (result >= 0 || errno != ENOSYS)
# endif
	{
	  if (oact && result >= 0)
	    {
	      oact->sa_handler = koact.k_sa_handler;
	      memcpy (&oact->sa_mask, &koact.sa_mask,
				sizeof (kernel_sigset_t));
	      oact->sa_flags = koact.sa_flags;
# ifdef HAVE_SA_RESTORER
	      oact->sa_restorer = koact.sa_restorer;
# endif
	    }
	  return result;
	}

# if __ASSUME_REALTIME_SIGNALS == 0
      __set_errno (saved_errno);
      __libc_missing_rt_sigs = 1;
# endif
    }
#endif

#if __ASSUME_REALTIME_SIGNALS == 0
  if (act)
    {
      k_sigact.k_sa_handler = act->sa_handler;
      k_sigact.sa_mask = act->sa_mask.__val[0];
      k_sigact.sa_flags = act->sa_flags;
# ifdef HAVE_SA_RESTORER
      k_sigact.sa_restorer = act->sa_restorer;
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
#  if _MIPS_SIM == _ABIO32
      oact->sa_restorer = k_osigact.sa_restorer;
#  else
      oact->sa_restorer = &restore;
#  endif
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

/* NOTE: Please think twice before making any changes to the bits of
   code below.  GDB needs some intimate knowledge about it to
   recognize them as signal trampolines, and make backtraces through
   signal handlers work right.  Important are both the names
   (__restore_rt) and the exact instruction sequence.
   If you ever feel the need to make any changes, please notify the
   appropriate GDB maintainer.  */

#define RESTORE(name, syscall) RESTORE2 (name, syscall)
#define RESTORE2(name, syscall) \
asm						\
  (						\
   ".align 4\n"					\
   "__" #name ":\n"				\
   "	li $2, " #syscall "\n"			\
   "	syscall\n"				\
   );

/* The return code for realtime-signals.  */
#if _MIPS_SIM != _ABIO32
# ifdef __NR_rt_sigreturn
RESTORE (restore_rt, __NR_rt_sigreturn)
# endif
# ifdef __NR_sigreturn
RESTORE (restore, __NR_sigreturn)
# endif
#endif
