/* Return from signal handler in GNU C library for Hurd.  Alpha version.
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

/* Declare global register variables before any code.  */
register double f0 asm ("$f0");
register double f1 asm ("$f1");
register double f2 asm ("$f2");
register double f3 asm ("$f3");
register double f4 asm ("$f4");
register double f5 asm ("$f5");
register double f6 asm ("$f6");
register double f7 asm ("$f7");
register double f8 asm ("$f8");
register double f9 asm ("$f9");
register double f10 asm ("$f10");
register double f11 asm ("$f11");
register double f12 asm ("$f12");
register double f13 asm ("$f13");
register double f14 asm ("$f14");
register double f15 asm ("$f15");
register double f16 asm ("$f16");
register double f17 asm ("$f17");
register double f18 asm ("$f18");
register double f19 asm ("$f19");
register double f20 asm ("$f20");
register double f21 asm ("$f21");
register double f22 asm ("$f22");
register double f23 asm ("$f23");
register double f24 asm ("$f24");
register double f25 asm ("$f25");
register double f26 asm ("$f26");
register double f27 asm ("$f27");
register double f28 asm ("$f28");
register double f29 asm ("$f29");
register double f30 asm ("$f30");;

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

  if (scp->sc_used_fpa)
    {
      /* Restore FPU state.  */

      /* Restore the floating-point control/status register.
	 We must do this first because the compiler will need
	 a temporary FP register for the load.  */
      asm volatile ("mt_fpcr %0" : : "f" (scp->sc_fpcsr));

      /* Restore floating-point registers. */
      f0 = scp->sc_fpregs[0];
      f1 = scp->sc_fpregs[1];
      f2 = scp->sc_fpregs[2];
      f3 = scp->sc_fpregs[3];
      f4 = scp->sc_fpregs[4];
      f5 = scp->sc_fpregs[5];
      f6 = scp->sc_fpregs[6];
      f7 = scp->sc_fpregs[7];
      f8 = scp->sc_fpregs[8];
      f9 = scp->sc_fpregs[9];
      f10 = scp->sc_fpregs[10];
      f11 = scp->sc_fpregs[11];
      f12 = scp->sc_fpregs[12];
      f13 = scp->sc_fpregs[13];
      f14 = scp->sc_fpregs[14];
      f15 = scp->sc_fpregs[15];
      f16 = scp->sc_fpregs[16];
      f17 = scp->sc_fpregs[17];
      f18 = scp->sc_fpregs[18];
      f19 = scp->sc_fpregs[19];
      f20 = scp->sc_fpregs[20];
      f21 = scp->sc_fpregs[21];
      f22 = scp->sc_fpregs[22];
      f23 = scp->sc_fpregs[23];
      f24 = scp->sc_fpregs[24];
      f25 = scp->sc_fpregs[25];
      f26 = scp->sc_fpregs[26];
      f27 = scp->sc_fpregs[27];
      f28 = scp->sc_fpregs[28];
      f29 = scp->sc_fpregs[29];
      f30 = scp->sc_fpregs[30];
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
    register integer_t *usp asm ("$3") = scpreg->sc_regs[30];
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
		  "call_pal %0"
		  : : "r" (rei_frame), "i" (op_rei));
    /* Firewall.  */
    asm volatile ("call_pal %0" : : "i" (op_halt));
  }

  /* NOTREACHED */
  return -1;
}
