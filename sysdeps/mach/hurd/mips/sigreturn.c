/* Copyright (C) 1991, 1992, 1994 Free Software Foundation, Inc.
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

  ss = _hurd_self_sigstate ();	/* SS->lock now locked.  */

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
      __mutex_unlock (&ss->lock);
      __sig_post (_hurd_msgport, 0, __mach_task_self ());
      /* If a pending signal was handled, sig_post never returned.  */
      __mutex_lock (&ss->lock);
    }

  if (scp->sc_onstack)
    {
      ss->sigaltstack.ss_flags &= ~SA_ONSTACK; /* XXX threadvars */
      /* XXX cannot unlock until off sigstack */
      abort ();
    }
  else
    __mutex_unlock (&ss->lock);

  /* Destroy the MiG reply port used by the signal handler, and restore the
     reply port in use by the thread when interrupted.  */
  reply_port =
    (mach_port_t *) __hurd_threadvar_location (_HURD_THREADVAR_MIG_REPLY);
  if (*reply_port)
    __mach_port_destroy (__mach_task_self (), *reply_port);
  *reply_port = scp->sc_reply_port;

  if (scp->sc_coproc_used & SC_COPROC_USE_FPU)
    {
      /* Restore FPU state.  */
#define restore_fpr(n) \
  asm volatile ("l.d $f" #n ",%0" : : "m" (scp->sc_fpr[n]))

      /* Restore floating-point registers. */
      restore_fpr (0);
      restore_fpr (2);
      restore_fpr (4);
      restore_fpr (6);
      restore_fpr (8);
      restore_fpr (10);
      restore_fpr (12);
      restore_fpr (14);
      restore_fpr (16);
      restore_fpr (18);
      restore_fpr (20);
      restore_fpr (22);
      restore_fpr (24);
      restore_fpr (26);
      restore_fpr (28);
      restore_fpr (30);

      /* Restore the floating-point control/status register ($f31).  */
      asm volatile ("ctc1 %0,$f31" : : "r" (scp->sc_fpcsr));
    }

  /* Load all the registers from the sigcontext.  */
#define restore_gpr(n) \
  asm volatile ("lw $" #n ",%0" : : "m" (scpreg->sc_gpr[n - 1]))

  {
    register const struct sigcontext *const scpreg asm ("$1") = scp;
    register int *at asm ("$1");

    /* First restore the multiplication result registers.  The compiler
       will use some temporary registers, so we do this before restoring
       the general registers.  */
    asm volatile ("mtlo %0" : : "r" (scpreg->sc_mdlo));
    asm volatile ("mthi %0" : : "r" (scpreg->sc_mdhi));

    /* In the word after the saved PC, store the saved $1 value.  */
    (&scpreg->sc_pc)[1] = scpreg->sc_gpr[0];

    asm volatile (".set noreorder; .set noat;");

    /* Restore the normal registers.  */
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
    /* Registers 26-27 are kernel-only.  */
    restore_gpr (28);
    restore_gpr (29);		/* Stack pointer.  */
    restore_gpr (30);		/* Frame pointer.  */
    restore_gpr (31);		/* Return address.  */

    at = &scpreg->sc_pc;
    /* This is an emulated instruction that will find at the address in $1
       two words: the PC value to restore, and the $1 value to restore.  */
    asm volatile (".word op_sigreturn");

    asm volatile (".set reorder; .set at;");
  }

  /* NOTREACHED */
  return -1;
}
