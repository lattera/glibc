/* POSIX.1 sigaction call for Linux/SPARC.
   Copyright (C) 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Miguel de Icaza (miguel@nuclecu.unam.mx), 1997.

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

#include <syscall.h>
#include <sys/signal.h>
#include <errno.h>

/* The kernel will deliver signals in the old way if the signal
   number is a positive number.  The kernel will deliver a signal
   with the new stack layout if the signal number is a negative number.

   Our sigaction code takes care of selecting the type of kernel we are
   using at runtime.  */

extern void ____sparc_signal_trampoline (int);
long ____sig_table [NSIG];

int
__trampoline_sigaction (int sig, struct sigaction *new, struct sigaction *old)
{
  int ret;
  int need_to_hide_trick = 0;
  __sighandler_t old_sh;

  if (new)
    {
      if (new->sa_handler != SIG_DFL && new->sa_handler != SIG_IGN)
	{
	  old_sh = ____sig_table[sig];
	  ____sig_table[sig] = (long int) new->sa_handler;
	  new->sa_handler = ____sparc_signal_trampoline;
	  need_to_hide_trick = 1;
	}
    }
  __asm__("or %%g0,%0,%%g1\n\t"
	  "or %%g0,%1,%%o0\n\t"
	  "or %%g0,%2,%%o1\n\t"
	  "or %%g0,%3,%%o2\n\t"
	  "t  0x10\n\t"
	  "bcc 1f\n\t"
	  "or %%o0, %%g0, %0\n\t"
	  "sub %%g0, %%o0, %0\n\t"
	  "1:"
	  : "=r" (ret), "=r" ((long int) sig), "=r" ((long int) new),
	    "=r" ((long int) old)
	  : "0" (SYS_sigaction), "1" (sig), "2" (new), "3" (old)
	  : "g1", "o0", "o1", "o2");

  if (ret >= 0)
    {
      if (old && old->sa_handler == ____sparc_signal_trampoline)
	{
	  if (need_to_hide_trick)
	    old->sa_handler = old_sh;
	  else
	    old->sa_handler = ____sig_table[sig];
	}
      if (need_to_hide_trick)
	new->sa_handler = ____sig_table[sig];
      return 0;
    }
  __set_errno (-ret);
  return -1;
}

int
__new_sigaction (int sig, struct sigaction *new, struct sigaction *old)
{
  int ret;

  sig = -sig;

  __asm__("or %%g0,%0,%%g1\n\t"
	  "or %%g0,%1,%%o0\n\t"
	  "or %%g0,%2,%%o1\n\t"
	  "or %%g0,%3,%%o2\n\t"
	  "t  0x10\n\t"
	  "bcc 1f\n\t"
	  "or %%o0, %%g0, %0\n\t"
	  "sub %%g0,%%o0,%0\n\t"
	  "1:"
	  : "=r" (ret), "=r" ((long int) sig), "=r" ((long int) new),
	    "=r" ((long int) old)
	  : "0" (SYS_sigaction), "1" (sig), "2" (new), "3" (old)
	  : "g1", "o0", "o1", "o2");
  if (ret >= 0)
    return 0;
  __set_errno (-ret);
  return -1;
}

int
__sigaction (int sig, struct sigaction *new, struct sigaction *old)
{
  static (*sigact_routine) (int, struct sigaction *, struct sigaction *);
  int ret;
  struct sigaction sa;

  if (sigact_routine)
    return (*sigact_routine) (sig, new, old);

  ret = __new_sigaction (1, NULL, &sa);
  if (ret == -1)
    sigact_routine = __trampoline_sigaction;
  else
    sigact_routine = __new_sigaction;

  return __sigaction (sig, new, old);
}
weak_alias (__sigaction, sigaction);
