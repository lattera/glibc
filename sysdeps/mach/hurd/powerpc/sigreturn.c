/* Return from signal handler for Hurd.  PowerPC version.
   Copyright (C) 1996,97,98,2001 Free Software Foundation, Inc.
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

#include <hurd.h>
#include <hurd/signal.h>
#include <hurd/threadvar.h>
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
      __spin_unlock (&ss->lock);
      __msg_sig_post (_hurd_msgport, 0, 0, __mach_task_self ());
      /* If a pending signal was handled, sig_post never returned.  */
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

  /* Restore FPU state.  */
#define restore_fpr(n) \
  asm volatile ("lfd " #n ",%0(31)" : : "i" (n * 4))

  asm volatile ("mr 31,%0" : : "r" (scp->sc_fprs));

  /* Restore the floating-point control/status register.  */
  asm volatile ("lfd 0,256(31)");
  asm volatile ("mtfsf 0xff,0");

  /* Restore floating-point registers. */
  restore_fpr (0);
  restore_fpr (1);
  restore_fpr (2);
  restore_fpr (3);
  restore_fpr (4);
  restore_fpr (5);
  restore_fpr (6);
  restore_fpr (7);
  restore_fpr (8);
  restore_fpr (9);
  restore_fpr (10);
  restore_fpr (11);
  restore_fpr (12);
  restore_fpr (13);
  restore_fpr (14);
  restore_fpr (15);
  restore_fpr (16);
  restore_fpr (17);
  restore_fpr (18);
  restore_fpr (19);
  restore_fpr (20);
  restore_fpr (21);
  restore_fpr (22);
  restore_fpr (23);
  restore_fpr (24);
  restore_fpr (25);
  restore_fpr (26);
  restore_fpr (27);
  restore_fpr (28);
  restore_fpr (29);
  restore_fpr (30);
  restore_fpr (31);

  /* Load all the registers from the sigcontext.  */
#define restore_gpr(n) \
  asm volatile ("lwz " #n ",%0(31)" : : "i" (n * 4))

  asm volatile ("addi 31,31,-188");  /* r31 = scp->gprs */

  /* Restore the special purpose registers.  */
  asm volatile ("lwz 0,128(31); mtcr 0");
  asm volatile ("lwz 0,132(31); mtxer 0");
  asm volatile ("lwz 0,136(31); mtlr 0");
  asm volatile ("lwz 0,-8(31); mtctr 0");  /* XXX this is the PC */
#if 0
  asm volatile ("lwz 0,144(31); mtmq %0");  /* PPC601 only */
#endif

  /* Restore the normal registers.  */
  restore_gpr (0);
  restore_gpr (1);
  restore_gpr (2);
  restore_gpr (3);
  restore_gpr (4);
  restore_gpr (5);
  restore_gpr (6);
  restore_gpr (7);
  restore_gpr (8);
  restore_gpr (9);
  restore_gpr (10);
  restore_gpr (11);
  restore_gpr (12);
  restore_gpr (13);
  restore_gpr (14);
  restore_gpr (15);
  restore_gpr (16);
  restore_gpr (17);
  restore_gpr (18);
  restore_gpr (19);
  restore_gpr (20);
  restore_gpr (21);
  restore_gpr (22);
  restore_gpr (23);
  restore_gpr (24);
  restore_gpr (25);
  restore_gpr (26);
  restore_gpr (27);
  restore_gpr (28);
  restore_gpr (29);
  restore_gpr (30);
  restore_gpr (31);

  /* Return. */
  asm volatile ("bctr");  /* XXX CTR is not restored! */

  /* NOTREACHED */
  return -1;
}

weak_alias (__sigreturn, sigreturn)
