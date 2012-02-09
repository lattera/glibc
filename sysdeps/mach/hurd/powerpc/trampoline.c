/* Set thread_state for sighandler, and sigcontext to recover.  For PowerPC.
   Copyright (C) 1994,1995,1996,1997,1998,1999,2001,2005
	Free Software Foundation, Inc.
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

#include <hurd/signal.h>
#include <hurd/userlink.h>
#include <thread_state.h>
#include <assert.h>
#include <errno.h>
#include "hurdfault.h"
#include <intr-msg.h>

struct sigcontext *
_hurd_setup_sighandler (struct hurd_sigstate *ss, __sighandler_t handler,
			int signo, struct hurd_signal_detail *detail,
			volatile int rpc_wait,
			struct machine_thread_all_state *state)
{
  void trampoline (void);
  void rpc_wait_trampoline (void);
  void *volatile sigsp;
  struct sigcontext *scp;

  if (ss->context)
    {
      /* We have a previous sigcontext that sigreturn was about
	 to restore when another signal arrived.  We will just base
	 our setup on that.  */
      if (! _hurdsig_catch_memory_fault (ss->context))
	{
	  memcpy (&state->basic, &ss->context->sc_ppc_thread_state,
		  sizeof (state->basic));
	  memcpy (&state->exc, &ss->context->sc_ppc_exc_state,
		  sizeof (state->exc));
	  memcpy (&state->fpu, &ss->context->sc_ppc_float_state,
		  sizeof (state->fpu));
	  state->set = (1 << PPC_THREAD_STATE) | (1 << PPC_EXCEPTION_STATE)
	    | (1 << PPC_FLOAT_STATE);
	}
    }

  if (! machine_get_basic_state (ss->thread, state))
    return NULL;

  if ((ss->actions[signo].sa_flags & SA_ONSTACK) &&
      !(ss->sigaltstack.ss_flags & (SS_DISABLE|SS_ONSTACK)))
    {
      sigsp = ss->sigaltstack.ss_sp + ss->sigaltstack.ss_size;
      ss->sigaltstack.ss_flags |= SS_ONSTACK;
      /* XXX need to set up base of new stack for
	 per-thread variables, cthreads.  */
    }
  else
    sigsp = (char *) state->basic.SP;

  /* Set up the sigcontext structure on the stack.  This is all the stack
     needs, since the args are passed in registers (below).  */
  sigsp -= sizeof (*scp);
  scp = sigsp;
  sigsp -= 16;  /* Reserve some space for a stack frame.  */

  if (_hurdsig_catch_memory_fault (scp))
    {
      /* We got a fault trying to write the stack frame.
	 We cannot set up the signal handler.
	 Returning NULL tells our caller, who will nuke us with a SIGILL.  */
      return NULL;
    }
  else
    {
      int ok;

      /* Set up the sigcontext from the current state of the thread.  */

      scp->sc_onstack = ss->sigaltstack.ss_flags & SS_ONSTACK ? 1 : 0;

      /* struct sigcontext is laid out so that starting at sc_srr0
	 mimics a struct ppc_thread_state.  */
      memcpy (&scp->sc_ppc_thread_state,
	      &state->basic, sizeof (state->basic));

      /* struct sigcontext is laid out so that starting at sc_dar
	 mimics a struct ppc_exc_state.  */
      ok = machine_get_state (ss->thread, state, PPC_EXCEPTION_STATE,
			       &state->exc, &scp->sc_ppc_exc_state,
			       sizeof (state->exc));

      /* struct sigcontext is laid out so that starting at sc_fprs[0]
	 mimics a struct ppc_float_state.  */
      if (ok)
	ok = machine_get_state (ss->thread, state, PPC_FLOAT_STATE,
				&state->fpu, &scp->sc_ppc_float_state,
				sizeof (state->fpu));

      _hurdsig_end_catch_fault ();

      if (!ok)
	return NULL;
    }

  /* Modify the thread state to call the trampoline code on the new stack.  */
  if (rpc_wait)
    {
      /* The signalee thread was blocked in a mach_msg_trap system call,
	 still waiting for a reply.  We will have it run the special
	 trampoline code which retries the message receive before running
	 the signal handler.

	 To do this we change the OPTION argument in its registers to
	 enable only message reception, since the request message has
	 already been sent.  */

      /* The system call arguments are stored in consecutive registers
	 starting with r3.  */
      struct mach_msg_trap_args *args = (void *) &state->basic.r3;

      if (_hurdsig_catch_memory_fault (args))
	{
	  /* Faulted accessing ARGS.  Bomb.  */
	  return NULL;
	}

      assert (args->option & MACH_RCV_MSG);
      /* Disable the message-send, since it has already completed.  The
	 calls we retry need only wait to receive the reply message.  */
      args->option &= ~MACH_SEND_MSG;

      /* Limit the time to receive the reply message, in case the server
	 claimed that `interrupt_operation' succeeded but in fact the RPC
	 is hung.  */
      args->option |= MACH_RCV_TIMEOUT;
      args->timeout = _hurd_interrupted_rpc_timeout;

      _hurdsig_end_catch_fault ();

      state->basic.PC = (int) rpc_wait_trampoline;
      /* After doing the message receive, the trampoline code will need to
	 update the r3 value to be restored by sigreturn.  To simplify
	 the assembly code, we pass the address of its slot in SCP to the
	 trampoline code in r10.  */
      state->basic.r10 = (long int) &scp->sc_gprs[3];
      /* We must preserve the mach_msg_trap args in r3..r9.
	 Pass the handler args to the trampoline code in r11..r13.  */
      state->basic.r11 = signo;
      state->basic.r12 = detail->code;
      state->basic.r13 = (int) scp;
    }
  else
    {
      state->basic.PC = (int) trampoline;
      state->basic.r3 = signo;
      state->basic.r4 = detail->code;
      state->basic.r5 = (int) scp;
    }

  state->basic.r1 = (int) sigsp;  /* r1 is the stack pointer.  */

  /* We pass the handler function to the trampoline code in ctr.  */
  state->basic.ctr = (int) handler;
  /* In r15, we store the address of __sigreturn itself,
     for the trampoline code to use.  */
  state->basic.r15 = (int) &__sigreturn;
  /* In r16, we save the SCP value to pass to __sigreturn
     after the handler returns.  */
  state->basic.r16 = (int) scp;

  /* In r3, we store a pointer to the registers in STATE so that the
     trampoline code can load the registers from that.  For some reason,
     thread_set_state doesn't set all registers.  */
  state->basic.r17 = state->basic.r3;  /* Store the real r3 in r17.  */
  state->basic.r3 = (int) &state->basic.r0;

  return scp;
}

/* The trampoline code follows.  This used to be located inside
   _hurd_setup_sighandler, but was optimized away by gcc 2.95.  */

/* This function sets some registers which the trampoline code uses
   and which are not automatically set by thread_set_state.
   In r3 we have a pointer to the registers in STATE.  */
asm ("trampoline_load_registers:\n"
     "lwz 17,68(3)\n"  /* The real r3.  */
     "lwz 4,16(3)\n"
     "lwz 5,20(3)\n"
     "lwz 6,24(3)\n"
     "lwz 7,28(3)\n"
     "lwz 8,32(3)\n"
     "lwz 9,36(3)\n"
     "lwz 10,40(3)\n"
     "lwz 11,44(3)\n"
     "lwz 12,48(3)\n"
     "lwz 13,52(3)\n"
     "lwz 14,56(3)\n"
     "lwz 15,60(3)\n"
     "lwz 16,64(3)\n"
     "mr 3,17\n"
     "blr\n");

asm ("rpc_wait_trampoline:\n");
  /* This is the entry point when we have an RPC reply message to receive
     before running the handler.  The MACH_MSG_SEND bit has already been
     cleared in the OPTION argument in our registers.  For our convenience,
     r10 points to the sc_regs[3] member of the sigcontext (saved r3).  */

asm (/* Retry the interrupted mach_msg system call.  */
     "bl trampoline_load_registers\n"
     "li 0, -25\n"		/* mach_msg_trap */
     "sc\n"
     /* When the sigcontext was saved, r3 was MACH_RCV_INTERRUPTED.  But
	now the message receive has completed and the original caller of
	the RPC (i.e. the code running when the signal arrived) needs to
	see the final return value of the message receive in r3.  So
	store the new r3 value into the sc_regs[3] member of the sigcontext
	(whose address is in r10 to make this code simpler).  */
     "stw 3, 0(10)\n"
     /* Since the argument registers needed to have the mach_msg_trap
	arguments, we've stored the arguments to the handler function
	in registers r11..r13 of the state structure.  */
     "mr 3,11\n"
     "mr 4,12\n"
     "mr 5,13\n");

asm ("trampoline:\n");
  /* Entry point for running the handler normally.  The arguments to the
     handler function are already in the standard registers:

       r3	SIGNO
       r4	SIGCODE
       r5	SCP

     r16 also contains SCP; this value is callee-saved (and so should not get
     clobbered by running the handler).  We use this saved value to pass to
     __sigreturn, so the handler can clobber the argument registers if it
     likes.  */
asm ("bl trampoline_load_registers\n"
     "bctrl\n"		/* Call the handler function.  */
     "mtctr 15\n"	/* Copy &__sigreturn to CTR.  */
     "mr 3,16\n"	/* Copy the saved SCP to r3.  */
     "bctr\n"		/* Call __sigreturn (SCP).  */
     );
