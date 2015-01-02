/* Copyright (C) 1991-2015 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

register int *sp asm ("%esp");

#include <hurd.h>
#include <hurd/signal.h>
#include <hurd/threadvar.h>
#include <hurd/msg.h>
#include <stdlib.h>
#include <string.h>

int
__sigreturn (struct sigcontext *scp)
{
  struct hurd_sigstate *ss;
  struct hurd_userlink *link = (void *) &scp[1];
  mach_port_t *reply_port;

  if (scp == NULL || (scp->sc_mask & _SIG_CANT_MASK))
    {
      errno = EINVAL;
      return -1;
    }

  ss = _hurd_self_sigstate ();
  __spin_lock (&ss->lock);

  /* Remove the link on the `active resources' chain added by
     _hurd_setup_sighandler.  Its purpose was to make sure
     that we got called; now we have, it is done.  */
  _hurd_userlink_unlink (link);

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
      __spin_unlock (&ss->lock);
      __msg_sig_post (_hurd_msgport, 0, 0, __mach_task_self ());
      /* If a pending signal was handled, sig_post never returned.
	 If it did return, the pending signal didn't run a handler;
	 proceed as usual.  */
      __spin_lock (&ss->lock);
      ss->context = NULL;
    }

  if (scp->sc_onstack)
    {
      ss->sigaltstack.ss_flags &= ~SS_ONSTACK; /* XXX threadvars */
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
    {
      mach_port_t port = *reply_port;

      /* Assigning MACH_PORT_DEAD here tells libc's mig_get_reply_port not to
	 get another reply port, but avoids mig_dealloc_reply_port trying to
	 deallocate it after the receive fails (which it will, because the
	 reply port will be bogus, whether we do this or not).  */
      *reply_port = MACH_PORT_DEAD;

      __mach_port_destroy (__mach_task_self (), port);
    }
  *reply_port = scp->sc_reply_port;

  if (scp->sc_fpused)
    /* Restore the FPU state.  Mach conveniently stores the state
       in the format the i387 `frstor' instruction uses to restore it.  */
    asm volatile ("frstor %0" : : "m" (scp->sc_fpsave));

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
