/* Set thread_state for sighandler, and sigcontext to recover.  MIPS version.
Copyright (C) 1994 Free Software Foundation, Inc.
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

#include <hurd/signal.h>
#include <mach/thread_status.h>

static void
trampoline (void (*handler) (int signo, int sigcode, struct sigcontext *scp),
	    int signo, int sigcode, struct sigcontext *scp)
{
  (*handler) (signo, sigcode, scp);
  (void) __sigreturn (scp);	/* Does not return.  */
  while (1)
    LOSE;			/* Firewall.  */
}

struct sigcontext *
_hurd_setup_sighandler (int flags,
			__sighandler_t handler,
			struct sigaltstack *sigaltstack,
			int signo, int sigcode,
			void *state)
{
  struct mips_thread_state *ts;
  void *sigsp;
  struct sigcontext *scp;

  ts = state;

  if ((flags & SA_ONSTACK) &&
      !(sigaltstack->ss_flags & (SA_DISABLE|SA_ONSTACK)))
    {
      sigsp = sigaltstack->ss_sp + sigaltstack->ss_size;
      sigaltstack->ss_flags |= SA_ONSTACK;
      /* XXX need to set up base of new stack for
	 per-thread variables, cthreads.  */
    }
  else
    sigsp = (char *) ts->r29;

  /* Set up the sigcontext structure on the stack.  This is all the stack
     needs, since the args are passed in registers (below).  */
  sigsp -= sizeof (*scp);
  scp = sigsp;

  /* Set up the sigcontext from the current state of the thread.  */

  scp->sc_onstack = sigaltstack->ss_flags & SA_ONSTACK ? 1 : 0;

  /* struct sigcontext is laid out so that starting at sc_gpr
     mimics a struct mips_thread_state.  */
  memcpy (scp->sc_gpr, &ts, sizeof ts);

  /* Modify the thread state to call `trampoline' on the new stack.  */

  /* These registers are used for passing the first four arguments to a
     function (the rest go on the stack).  Fortunately `trampoline' takes
     just four arguments, so they all fit in registers.  */
  ts->r4 = (int) handler;
  ts->r5 = signo;
  ts->r6 = sigcode;
  ts->r7 = (int) scp;

  ts->r29 = (int) sigsp;	/* r29 is the stack pointer register.  */
  ts->pc = (int) &trampoline;

  return scp;
}
