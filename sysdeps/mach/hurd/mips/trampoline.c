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
#include "thread_state.h"


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
			int signo, int sigcode,
			int rpc_wait,
			struct machine_thread_all_state *state)
{

  __label__ trampoline, rpc_wait_trampoline;
  void *sigsp;
  struct sigcontext *scp;

  if (ss->context)
    {
      /* We have a previous sigcontext that sigreturn was about
	 to restore when another signal arrived.  We will just base
	 our setup on that.  */
      if (! setjmp (_hurd_sigthread_fault_env))
	{
	  memcpy (&state->basic, &ss->context->sc_mips_thread_state,
		  sizeof (state->basic));
	  memcpy (&state->exc, &ss->context->sc_mips_exc_state,
		  sizeof (state->exc));
	  state->set = (1 << MIPS_THREAD_STATE) | (1 << MIPS_EXC_STATE);
	  if (state->exc.coproc_state & SC_COPROC_USE_FPU)
	    {
	      memcpy (&state->fpu, &ss->context->sc_mips_loat_state,
		      sizeof (state->fpu));
	      state->set |= (1 << MIPS_FLOAT_STATE);
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
      /* If the sigreturn context was bogus, just ignore it.  */
      ss->context = NULL;
    }
  else if (! machine_get_basic_state (ss->thread, state))
    return NULL;

  if ((ss->actions[signo].sa_flags & SA_ONSTACK) &&
      !(ss->sigaltstack.ss_flags & (SA_DISABLE|SA_ONSTACK)))
    {
      sigsp = ss->sigaltstack.ss_sp + ss->sigaltstack.ss_size;
      ss->sigaltstack.ss_flags |= SA_ONSTACK;
      /* XXX need to set up base of new stack for
	 per-thread variables, cthreads.  */
    }
  else
    sigsp = (char *) state->basic.r29;

  /* Set up the sigcontext structure on the stack.  This is all the stack
     needs, since the args are passed in registers (below).  */
  sigsp -= sizeof (*scp);
  scp = sigsp;

  if (! setjmp (_hurd_sigthread_fault_env))
    {
      /* Set up the sigcontext from the current state of the thread.  */

      scp->sc_onstack = ss->sigaltstack.ss_flags & SA_ONSTACK ? 1 : 0;

      /* struct sigcontext is laid out so that starting at sc_gpr
	 mimics a struct mips_thread_state.  */
      memcpy (&scp->sc_mips_thread_state,
	      &state->basic, sizeof (state->basic));

      /* struct sigcontext is laid out so that starting at sc_cause
	 mimics a struct mips_exc_state.  */
      if (! machine_get_state (ss->thread, state, MIPS_EXC_STATE,
			       &state->exc, &scp->sc_cause,
			       sizeof (state->exc)))
	return NULL;
      if ((scp->sc_coproc_used & SC_COPROC_USE_FPU) &&
	  /* struct sigcontext is laid out so that starting at sc_fpr
	     mimics a struct mips_float_state.  This state
	     is only meaningful if the coprocessor was used.  */
	  ! machine_get_state (ss->thread, state, MIPS_FLOAT_STATE,
			       &state->fpu,
			       &scp->sc_mips_float_state, sizeof (state->fpu)))
	return NULL;
    }
  else
    /* We got a fault trying to write the stack frame.
       We cannot set up the signal handler.
       Returning NULL tells our caller, who will nuke us with a SIGILL.  */
    return NULL;

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
	 starting with a0 ($4).  */
      struct mach_msg_trap_args *args = (void *) &state->basic.r4;

      assert (args->option & MACH_RCV_MSG);
      /* Disable the message-send, since it has already completed.  The
	 calls we retry need only wait to receive the reply message.  */
      args->option &= ~MACH_SEND_MSG;

      state->basic.pc = (int) &&rpc_wait_trampoline;
      state->basic.r29 = (int) sigsp; /* $29 is the stack pointer register.  */
      /* After doing the message receive, the trampoline code will need to
	 update the v0 ($2) value to be restored by sigreturn.  To simplify
	 the assembly code, we pass the address of its slot in SCP to the
	 trampoline code in v1 ($3).  */
      state->basic.r3 = (int) &scp->sc_gpr[1];
      /* We must preserve the mach_msg_trap args in a0..t2 ($4..$10).
	 Pass the handler args to the trampoline code in s1..s3 ($17..$19).  */
      state->basic.r17 = signo;
      state->basic.r18 = sigcode;
      state->basic.r19 = (int) scp;
    }
  else
    {
      state->basic.pc = (int) &&trampoline;
      state->basic.r29 = (int) sigsp;
      state->basic.r4 = signo;
      state->basic.r5 = sigcode;
      state->basic.r6 = (int) scp;
    }

  /* We pass the handler function to the trampoline code in at ($1).  */
  state->basic.r1 = (int) handler;
  /* In the callee-saved register s0 ($16), we save the SCP value to pass
     to __sigreturn after the handler returns.  */
  state->basic.r16 = (int) scp;

  return scp;

  /* The trampoline code follows.  This is not actually executed as part of
     this function, it is just convenient to write it that way.  */

 rpc_wait_trampoline:
  /* This is the entry point when we have an RPC reply message to receive
     before running the handler.  The MACH_MSG_SEND bit has already been
     cleared in the OPTION argument in our registers.  For our convenience,
     $3 points to the sc_gpr[1] member of the sigcontext (saved v0 ($2)).  */
  asm volatile
    (".set noat; .set noreorder; .set nomacro\n"
     /* Retry the interrupted mach_msg system call.  */
     "li v0, -25\n"		/* mach_msg_trap */
     "syscall\n"
     /* When the sigcontext was saved, v0 was MACH_RCV_INTERRUPTED.  But
	now the message receive has completed and the original caller of
	the RPC (i.e. the code running when the signal arrived) needs to
	see the final return value of the message receive in v0.  So
	store the new v0 value into the sc_gpr[1] member of the sigcontext
	(whose address is in v1 to make this code simpler).  */
     "sw v0, (v1)\n"
     /* Since the argument registers needed to have the mach_msg_trap
	arguments, we've stored the arguments to the handler function
	in registers s1..s3 ($17..$19).  */
     "move a0, s1\n"
     "move a1, s2\n"
     "move a2, s3\n");

 trampoline:
  /* Entry point for running the handler normally.  The arguments to the
     handler function are already in the standard registers:

       a0	SIGNO
       a1	SIGCODE
       a2	SCP
     */
  asm volatile
    ("jal $1; nop\n"		/* Call the handler function.  */
     /* Call __sigreturn (SCP); this cannot return.  */
     "j %0\n"
     "move a0, s0"		/* Set up arg from saved SCP in delay slot.  */
     : : "i" (&__sigreturn));

  /* NOTREACHED */
  asm volatile (".set reorder; .set at; .set macro");

  return NULL;
}

/* STATE describes a thread that had intr_port set (meaning it was inside
   HURD_EINTR_RPC), after it has been thread_abort'd.  It it looks to have
   just completed a mach_msg_trap system call that returned
   MACH_RCV_INTERRUPTED, return nonzero and set *PORT to the receive right
   being waited on.  */
int
_hurdsig_rcv_interrupted_p (struct machine_thread_all_state *state,
			    mach_port_t *port)
{
  if (! setjmp (_hurd_sigthread_fault_env))
    {
      const unsigned int *pc = (void *) state->basic.pc;
      if (state->basic.r2 == MACH_RCV_INTERRUPTED &&
	  pc[-1] == 0xc)	/* syscall */
	{
	  /* We did just return from a mach_msg_trap system call
	     doing a message receive that was interrupted.
	     Examine the parameters to find the receive right.  */
	  struct mach_msg_trap_args *args = (void *) &state->basic.r4;

	  *port = args->rcv_name;
	  return 1;
	}
    }

  return 0;
}
