/* Copyright (C) 1996, 1997, 1998 Free Software Foundation, Inc.
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
#include <mach/mips/mips_instruction.h>

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

  if (scp->sc_coproc_used & SC_COPROC_USE_FPU)
    {
      /* Restore FPU state.  */
#define restore_fpr(n) \
  asm volatile ("l.d $f" #n ",%0" : : "m" (scp->sc_fpr[n]))

      /* Restore floating-point registers. */
#ifdef __mips64
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
#else
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
#endif

      /* Restore the floating-point control/status register ($f31).  */
      asm volatile ("ctc1 %0,$f31" : : "r" (scp->sc_fpcsr));
    }

  /* Load all the registers from the sigcontext.  */
#ifdef __mips64
#define restore_gpr(n) \
  asm volatile ("ld $" #n ",%0" : : "m" (scpreg->sc_gpr[n - 1]))
#else
#define restore_gpr(n) \
  asm volatile ("lw $" #n ",%0" : : "m" (scpreg->sc_gpr[n - 1]))
#endif

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
    asm volatile (".word %0" : : "i" (op_sigreturn));
    asm volatile (".set reorder; .set at;");
    /* NOTREACHED */
    return at;		/* To prevent optimization.  */
  }

  /* NOTREACHED */
  return -1;
}

weak_alias (__sigreturn, sigreturn)
