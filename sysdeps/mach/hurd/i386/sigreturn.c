/* Copyright (C) 1991, 1992, 1994, 1995 Free Software Foundation, Inc.
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

register int *sp asm ("%esp");

#include <hurd.h>
#include <hurd/signal.h>
#include <hurd/threadvar.h>
#include <hurd/msg.h>
#include <stdlib.h>

int
__sigreturn (struct sigcontext *scp)
{
  struct hurd_sigstate *ss;
  mach_port_t *reply_port;

  if (scp == NULL || (scp->sc_mask & _SIG_CANT_MASK))
    {
      errno = EINVAL;
      return -1;
    }

  ss = _hurd_self_sigstate ();
  __spin_lock (&ss->lock);

  /* Restore the set of blocked signals, and the intr_port slot.  */
  ss->blocked = scp->sc_mask;
  ss->intr_port = scp->sc_intr_port;

  /* Check for pending signals that were blocked by the old set.  */
  if (ss->pending & ~ss->blocked)
    {
      /* There are pending signals that just became unblocked.  Wake up the
	 signal thread to deliver them.  But first, squirrel away SCP where
	 the signal thread will notice it if it runs another handler, and
	 arrange to have us called over again in the new reality.  */
      ss->context = scp;
      /* Clear the intr_port slot, since we are not in fact doing
	 an interruptible RPC right now.  If SS->intr_port is not null,
	 the SCP context is doing an interruptible RPC, but the signal
	 thread will examine us while we are blocked in the sig_post RPC.  */
      ss->intr_port = MACH_PORT_NULL;
      __spin_unlock (&ss->lock);
      __msg_sig_post (_hurd_msgport, 0, __mach_task_self ());
      /* If a pending signal was handled, sig_post never returned.  */
      __spin_lock (&ss->lock);
    }

  if (scp->sc_onstack)
    {
      ss->sigaltstack.ss_flags &= ~SA_ONSTACK; /* XXX threadvars */
      /* XXX cannot unlock until off sigstack */
      abort ();
    }
  else
    __spin_unlock (&ss->lock);

  /* Destroy the MiG reply port used by the signal handler, and restore the
     reply port in use by the thread when interrupted.  */
  reply_port =
    (mach_port_t *) __hurd_threadvar_location (_HURD_THREADVAR_MIG_REPLY);
  if (*reply_port)
    __mach_port_destroy (__mach_task_self (), *reply_port);
  *reply_port = scp->sc_reply_port;

  if (scp->sc_fpused)
    {
  /* XXX should restore FPU state here XXX roland needs 387 manual */
  /*    abort (); */
    }

  {
    /* There are convenient instructions to pop state off the stack, so we
       copy the registers onto the user's stack, switch there, pop and
       return.  */

    int *usp = (int *) scp->sc_uesp;

    *--usp = scp->sc_eip;
    *--usp = scp->sc_efl;
    memcpy (usp -= 12, &scp->sc_i386_thread_state, 12 * sizeof (int));

    sp = usp;

#define A(line) asm volatile (#line)
    /* The members in the sigcontext are arranged in this order
       so we can pop them easily.  */

    /* Pop the segment registers (except %cs and %ss, done last).  */
    A (popl %gs);
    A (popl %fs);
    A (popl %es);
    A (popl %ds);
    /* Pop the general registers.  */
    A (popa);
    /* Pop the processor flags.  */
    A (popf);
    /* Return to the saved PC.  */
    A (ret);

    /* Firewall.  */
    A (hlt);
#undef A
  }

  /* NOTREACHED */
  return -1;
}

weak_alias (__sigreturn, sigreturn)
