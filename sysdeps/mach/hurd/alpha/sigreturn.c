/* Return from signal handler in GNU C library for Hurd.  Alpha version.
   Copyright (C) 1994,95,97,98,2002 Free Software Foundation, Inc.
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
#include <hurd/msg.h>
#include <stdlib.h>
#include <string.h>

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
      __msg_sig_post (_hurd_msgport, 0, 0, __mach_task_self ());
      /* If a pending signal was handled, sig_post never returned.  */
      __spin_lock (&ss->lock);
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
    __mach_port_destroy (__mach_task_self (), *reply_port);
  *reply_port = scp->sc_reply_port;

  if (scp->sc_used_fpa)
    {
      /* Restore FPU state.  */

      /* Restore the floating-point control/status register.
	 We must do this first because the compiler will need
	 a temporary FP register for the load.  */
      asm volatile ("mt_fpcr %0" : : "f" (scp->sc_fpcsr));

      /* Restore floating-point registers. */
#define restore_fpr(n) \
  asm volatile ("ldt $f" #n ",%0" : : "m" (scp->sc_fpregs[n]))
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
    }

  /* Load all the registers from the sigcontext.  */
#define restore_gpr(n) \
  asm volatile ("ldq $" #n ",%0" : : "m" (scpreg->sc_regs[n]))

  {
    /* The `rei' PAL pseudo-instruction restores registers $2..$7, the PC
       and processor status.  So we can use these few registers for our
       working variables.  Unfortunately, it finds its data on the stack
       and merely pops the SP ($30) over the words of state restored,
       allowing no other option for the new SP value.  So we must push the
       registers and PSW it will to restore, onto the user's stack and let
       it pop them from there.  */
    register const struct sigcontext *const scpreg asm ("$2") = scp;
    register integer_t *usp asm ("$3") = (integer_t *) scpreg->sc_regs[30];
    register integer_t usp_align asm ("$4");

    /* Push an 8-word "trap frame" onto the user stack for `rei':
       registers $2..$7, the PC, and the PSW.  */

    register struct rei_frame
      {
	integer_t regs[5], pc, ps;
      } *rei_frame asm ("$5");

    usp -= 8;
    /* `rei' demands that the stack be aligned to a 64 byte (8 word)
       boundary; bits 61..56 of the PSW are OR'd back into the SP value
       after popping the 8-word trap frame, so we store (sp % 64)
       there and this restores the original user SP.  */
    usp_align = (integer_t) usp & 63L;
    rei_frame = (void *) ((integer_t) usp & ~63L);

    /* Copy the registers and PC from the sigcontext.  */
    memcpy (rei_frame->regs, &scpreg->sc_regs[2], sizeof rei_frame->regs);
    rei_frame->pc = scpreg->sc_pc;

    /* Compute the new PS value to be restored.  `rei' adds the value at
       bits 61..56 to the SP to compensate for the alignment above that
       cleared the low 6 bits; bits 5..3 are the new mode/privilege level
       (must be >= current mode; 3 == user mode); bits 2..0 are "software",
       unused by the processor or kernel (XXX should trampoline save these?
       How?); in user mode, `rei' demands that all other bits be zero.  */
    rei_frame->ps = (usp_align << 56) | (3 << 3); /* XXX low 3 bits??? */

    /* Restore the other general registers: everything except $2..$7, which
       are in the `rei' trap frame we set up above, and $30, which is the
       SP which is popped by `rei'.  */
    restore_gpr (1);
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

    /* Switch the stack pointer to the trap frame set up on
       the user stack and do the magical `rei' PAL call.  */
    asm volatile ("mov %0, $30\n"
		  "call_pal %1"
		  : : "r" (rei_frame), "i" (63)); /* PAL_rti */
    /* Firewall.  */
    asm volatile ("halt");
  }

  /* NOTREACHED */
  return -1;
}

weak_alias (__sigreturn, sigreturn)
