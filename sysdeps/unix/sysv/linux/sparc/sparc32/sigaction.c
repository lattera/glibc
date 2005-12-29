/* POSIX.1 sigaction call for Linux/SPARC.
   Copyright (C) 1997-2000,2002,2003,2005 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Miguel de Icaza (miguel@nuclecu.unam.mx), 1997.

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

#include <string.h>
#include <syscall.h>
#include <sys/signal.h>
#include <errno.h>
#include <kernel_sigaction.h>
#include <sysdep.h>

static void __rt_sigreturn_stub (void);
static void __sigreturn_stub (void);

/* The variable is shared between all wrappers around signal handling
   functions which have RT equivalents.  */
int __libc_missing_rt_sigs;

int
__libc_sigaction (int sig, __const struct sigaction *act,
		  struct sigaction *oact)
{
  struct old_kernel_sigaction k_sigact, k_osigact;
  int ret;

#ifdef __NR_rt_sigaction
  /* First try the RT signals.  */
  if (!__libc_missing_rt_sigs)
    {
      struct kernel_sigaction kact, koact;
      unsigned long stub = 0;
      int saved_errno = errno;

      if (act)
	{
	  kact.k_sa_handler = act->sa_handler;
	  memcpy (&kact.sa_mask, &act->sa_mask, sizeof (sigset_t));
	  if (((kact.sa_flags = act->sa_flags) & SA_SIGINFO) != 0)
	    stub = (unsigned long) &__rt_sigreturn_stub;
	  else
	    stub = (unsigned long) &__sigreturn_stub;
	  stub -= 8;
	  kact.sa_restorer = NULL;
	}

      /* XXX The size argument hopefully will have to be changed to the
	 real size of the user-level sigset_t.  */
      ret = INLINE_SYSCALL (rt_sigaction, 5, sig, act ? &kact : 0,
			    oact ? &koact : 0, stub, _NSIG / 8);

      if (ret >= 0 || errno != ENOSYS)
	{
	  if (oact && ret >= 0)
	    {
	      oact->sa_handler = koact.k_sa_handler;
	      memcpy (&oact->sa_mask, &koact.sa_mask, sizeof (sigset_t));
	      oact->sa_flags = koact.sa_flags;
	      oact->sa_restorer = koact.sa_restorer;
	    }
	  return ret;
	}

      __set_errno (saved_errno);
      __libc_missing_rt_sigs = 1;
    }
#endif

  /* Magic to tell the kernel we are using "new-style" signals, in that
     the signal table is not kept in userspace.  Not the same as the
     really-new-style rt signals.  */
  sig = -sig;

  if (act)
    {
      k_sigact.k_sa_handler = act->sa_handler;
      k_sigact.sa_mask = act->sa_mask.__val[0];
      k_sigact.sa_flags = act->sa_flags;
      k_sigact.sa_restorer = NULL;
    }

  {
    register int r_syscallnr __asm__("%g1") = __NR_sigaction;
    register int r_sig __asm__("%o0") = sig;
    register struct old_kernel_sigaction *r_act __asm__("%o1");
    register struct old_kernel_sigaction *r_oact __asm__("%o2");

    r_act = act ? &k_sigact : NULL;
    r_oact = oact ? &k_osigact : NULL;

    __asm__ __volatile__("t 0x10\n\t"
			 "bcc 1f\n\t"
			 " nop\n\t"
			 "sub %%g0,%%o0,%%o0\n"
			 "1:"
			 : "=r"(r_sig)
			 : "r"(r_syscallnr), "r"(r_act), "r"(r_oact),
			   "0"(r_sig));

    ret = r_sig;
  }

  if (ret >= 0)
    {
      if (oact)
	{
	  oact->sa_handler = k_osigact.k_sa_handler;
	  oact->sa_mask.__val[0] = k_osigact.sa_mask;
	  oact->sa_flags = k_osigact.sa_flags;
	  oact->sa_restorer = NULL;
	}
      return ret;
    }

  __set_errno (-ret);
  return -1;
}
libc_hidden_def (__libc_sigaction)

#ifdef WRAPPER_INCLUDE
# include WRAPPER_INCLUDE
#endif

#ifndef LIBC_SIGACTION
weak_alias (__libc_sigaction, __sigaction);
libc_hidden_weak (__sigaction)
weak_alias (__libc_sigaction, sigaction);
#endif

static void
__rt_sigreturn_stub (void)
{
  __asm__ ("mov %0, %%g1\n\t"
	   "ta	0x10\n\t"
	   : /* no outputs */
	   : "i" (__NR_rt_sigreturn));
}

static void
__sigreturn_stub (void)
{
  __asm__ ("mov %0, %%g1\n\t"
	   "ta	0x10\n\t"
	   : /* no outputs */
	   : "i" (__NR_sigreturn));
}
