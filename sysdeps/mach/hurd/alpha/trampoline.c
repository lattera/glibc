/* Set thread_state for sighandler, and sigcontext to recover.  Alpha version.
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

#include <hurd/signal.h>
#include "thread_state.h"
#include "hurdfault.h"
#include <assert.h>

struct mach_msg_trap_args
  {
    /* This is the order of arguments to mach_msg_trap.  */
    mach_msg_header_t *msg;
    mach_msg_option_t option;
    mach_msg_size_t send_size;
    mach_msg_size_t rcv_size;
    mach_port_t rcv_name;
    mach_msg_timeout_t timeout;
    mach_port_t notify;
  };


struct sigcontext *
_hurd_setup_sighandler (struct hurd_sigstate *ss, __sighandler_t handler,
			int signo, struct hurd_signal_detail *detail,
			int rpc_wait, struct machine_thread_all_state *state)
{
  __label__ trampoline, rpc_wait_trampoline;
  void *sigsp;
  struct sigcontext *scp;

  if (ss->context)
    {
      /* We have a previous sigcontext that sigreturn was about
	 to restore when another signal arrived.  We will just base
	 our setup on that.  */
      if (! _hurdsig_catch_memory_fault (ss->context))
	{
	  memcpy (&state->basic, &ss->context->sc_alpha_thread_state,
		  sizeof (state->basic));
	  memcpy (&state->exc, &ss->context->sc_alpha_exc_state,
		  sizeof (state->exc));
	  state->set = (1 << ALPHA_THREAD_STATE) | (1 << ALPHA_EXC_STATE);
	  if (state->exc.used_fpa)
	    {
	      memcpy (&state->fpu, &ss->context->sc_alpha_float_state,
		      sizeof (state->fpu));
	      state->set |= (1 << ALPHA_FLOAT_STATE);
	    }
	  assert (! rpc_wait);
	  /* The intr_port slot was cleared before sigreturn sent us the
	     sig_post that made us notice this pending signal, so
	     _hurd_internal_post_signal wouldn't do interrupt_operation.
	     After we return, our caller will set SCP->sc_intr_port (in the
	     new context) from SS->intr_port and clear SS->intr_port.  Now
	     that we are restoring this old context recorded by sigreturn,
	     we want to restore its intr_port too; so store it in
	     SS->intr_port now, so it will end up in SCP->sc_intr_port
	     later.  */
	  ss->intr_port = ss->context->sc_intr_port;
	}
      _hurdsig_end_catch_fault ();

      /* If the sigreturn context was bogus, just ignore it.  */
      ss->context = NULL;
    }
  else if (! machine_get_basic_state (ss->thread, state))
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

  if (_hurdsig_catch_memory_fault (scp))
    {
      /* We got a fault trying to write the stack frame.
	 We cannot set up the signal handler.
	 Returning NULL tells our caller, who will nuke us with a SIGILL.  */
      return NULL;
    }
  else
    {
      /* Set up the sigcontext from the current state of the thread.  */

      scp->sc_onstack = ss->sigaltstack.ss_flags & SS_ONSTACK ? 1 : 0;

      /* struct sigcontext is laid out so that starting at sc_regs
	 mimics a struct alpha_thread_state.  */
      memcpy (&scp->sc_alpha_thread_state,
	      &state->basic, sizeof (state->basic));

      /* struct sigcontext is laid out so that starting at sc_badvaddr
	 mimics a struct mips_exc_state.  */
      if (! machine_get_state (ss->thread, state, ALPHA_EXC_STATE,
			       &state->exc, &scp->sc_alpha_exc_state,
			       sizeof (state->exc)))
	return NULL;

      if (state->exc.used_fpa &&
	  /* struct sigcontext is laid out so that starting at sc_fpregs
	     mimics a struct alpha_float_state.  This state
	     is only meaningful if the coprocessor was used.  */
	  ! machine_get_state (ss->thread, state, ALPHA_FLOAT_STATE,
			       &state->fpu,
			       &scp->sc_alpha_float_state,
			       sizeof (state->fpu)))
	return NULL;

      _hurdsig_end_catch_fault ();
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
	 starting with a0 ($16).  */
      struct mach_msg_trap_args *args = (void *) &state->basic.r16;

      assert (args->option & MACH_RCV_MSG);
      /* Disable the message-send, since it has already completed.  The
	 calls we retry need only wait to receive the reply message.  */
      args->option &= ~MACH_SEND_MSG;

      /* Limit the time to receive the reply message, in case the server
	 claimed that `interrupt_operation' succeeded but in fact the RPC
	 is hung.  */
      args->option |= MACH_RCV_TIMEOUT;
      args->timeout = _hurd_interrupted_rpc_timeout;

      state->basic.pc = (long int) &&rpc_wait_trampoline;
      /* After doing the message receive, the trampoline code will need to
	 update the v0 ($0) value to be restored by sigreturn.  To simplify
	 the assembly code, we pass the address of its slot in SCP to the
	 trampoline code in at ($28).  */
      state->basic.r28 = (long int) &scp->sc_regs[0];
      /* We must preserve the mach_msg_trap args in a0..a5 and t0
	 ($16..$21, $1).  Pass the handler args to the trampoline code in
	 t8..t10 ($22.$24).  */
      state->basic.r22 = signo;
      state->basic.r23 = detail->code;
      state->basic.r24 = (long int) scp;
    }
  else
    {
      state->basic.pc = (long int) &&trampoline;
      state->basic.r16 = signo;
      state->basic.r17 = detail->code;
      state->basic.r18 = (long int) scp;
    }

  state->basic.r30 = (long int) sigsp; /* $30 is the stack pointer.  */

  /* We pass the handler function to the trampoline code in ra ($26).  */
  state->basic.r26 = (long int) handler;
  /* In the callee-saved register t12/pv ($27), we store the
     address of __sigreturn itself, for the trampoline code to use.  */
  state->basic.r27 = (long int) &__sigreturn;
  /* In the callee-saved register t11/ai ($25), we save the SCP value to pass
     to __sigreturn after the handler returns.  */
  state->basic.r25 = (long int) scp;

  return scp;

  /* The trampoline code follows.  This is not actually executed as part of
     this function, it is just convenient to write it that way.  */

 rpc_wait_trampoline:
  /* This is the entry point when we have an RPC reply message to receive
     before running the handler.  The MACH_MSG_SEND bit has already been
     cleared in the OPTION argument in our registers.  For our convenience,
     at ($28) points to the sc_regs[0] member of the sigcontext (saved v0
     ($0)).  */
  asm volatile
    (/* Retry the interrupted mach_msg system call.  */
     "lda $0, -25($31)\n"	/* mach_msg_trap */
     "callsys\n"		/* Magic system call instruction.  */
     /* When the sigcontext was saved, v0 was MACH_RCV_INTERRUPTED.  But
	now the message receive has completed and the original caller of
	the RPC (i.e. the code running when the signal arrived) needs to
	see the final return value of the message receive in v0.  So
	store the new v0 value into the sc_regs[0] member of the sigcontext
	(whose address is in at to make this code simpler).  */
     "stq $0, 0($28)\n"
     /* Since the argument registers needed to have the mach_msg_trap
	arguments, we've stored the arguments to the handler function
	in registers t8..t10 ($22..$24).  */
     "mov $22, $16\n"
     "mov $23, $17\n"
     "mov $24, $18\n");

 trampoline:
  /* Entry point for running the handler normally.  The arguments to the
     handler function are already in the standard registers:

       a0	SIGNO
       a1	SIGCODE
       a2	SCP

     t12 also contains SCP; this value is callee-saved (and so should not get
     clobbered by running the handler).  We use this saved value to pass to
     __sigreturn, so the handler can clobber the argument registers if it
     likes.  */
  /* Call the handler function, saving return address in ra ($26).  */
  asm volatile ("jsr $26, ($26)");
  /* Reset gp ($29) from the return address (here) in ra ($26).  */
  asm volatile ("ldgp $29, 0($26)");
  asm volatile ("mov $25, $16"); /* Move saved SCP to argument register.  */
  /* Call __sigreturn (SCP); this cannot return.  */
  asm volatile ("jmp $31, ($27)");

  /* NOTREACHED */
  return NULL;
}
