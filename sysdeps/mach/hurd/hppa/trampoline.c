/* Set thread_state for sighandler, and sigcontext to recover.  HPPA version.
   Copyright (C) 1995, 1997, 1998 Free Software Foundation, Inc.
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
#include <assert.h>
#include <errno.h>
#include "hurdfault.h"


struct mach_msg_trap_regargs
  {
    /* These first four arguments are in registers 26..23.  */
    mach_msg_size_t rcv_size;	/* arg3 */
    mach_msg_size_t send_size;	/* arg2 */
    mach_msg_option_t option;	/* arg1 */
    mach_msg_header_t *msg;	/* arg0 */
  };

struct sigcontext *
_hurd_setup_sighandler (struct hurd_sigstate *ss, __sighandler_t handler,
			int signo, long int sigcode,
			volatile int rpc_wait,
			struct machine_thread_all_state *state)
{
  __label__ trampoline, rpc_wait_trampoline;
  void *volatile sigsp;
  struct sigcontext *scp;

  if (ss->context)
    {
      /* We have a previous sigcontext that sigreturn was about
	 to restore when another signal arrived.  We will just base
	 our setup on that.  */
      if (_hurdsig_catch_fault (SIGSEGV))
	assert (_hurdsig_fault_sigcode >= (long int) ss->context &&
		_hurdsig_fault_sigcode < (long int) (ss->context + 1));
      else
	{
	  memcpy (&state->basic, &ss->context->sc_parisc_thread_state,
		  sizeof (state->basic));
	  state->set = (1 << PARISC_THREAD_STATE);
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
      !(ss->sigaltstack.ss_flags & (SS_DISABLE|SS_ONSTACK)))
    {
      sigsp = ss->sigaltstack.ss_sp + ss->sigaltstack.ss_size;
      ss->sigaltstack.ss_flags |= SS_ONSTACK;
      /* XXX need to set up base of new stack for
	 per-thread variables, cthreads.  */
    }
  else
    sigsp = (char *) state->basic.uesp;

  /* Push the signal context on the stack.  */
  sigsp -= sizeof (*scp);
  scp = sigsp;

  if (_hurdsig_catch_fault (SIGSEGV))
    {
      assert (_hurdsig_fault_sigcode >= (long int) scp &&
	      _hurdsig_fault_sigcode <= (long int) (scp + 1));
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

      /* struct sigcontext is laid out so that starting at sc_regs mimics a
	 struct parisc_thread_state.  */
      memcpy (&scp->sc_parisc_thread_state,
	      &state->basic, sizeof (state->basic));

      _hurdsig_end_catch_fault ();

      if (! ok)
	return NULL;
    }

  /* Modify the thread state to call the trampoline code on the new stack.  */
  if (rpc_wait)
    {
      /* The signalee thread was blocked in a mach_msg_trap system call,
	 still waiting for a reply.  We will have it run the special
	 trampoline code which retries the message receive before running
	 the signal handler.

	 To do this we change the OPTION argument on its stack to enable only
	 message reception, since the request message has already been
	 sent.  */

      struct mach_msg_trap_regargs *args = (void *) &state->basic.r23;

      if (_hurdsig_catch_fault (SIGSEGV))
	{
	  assert (_hurdsig_fault_sigcode >= (long int) args &&
		  _hurdsig_fault_sigcode < (long int) (args + 1));
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

      MACHINE_THREAD_STATE_SET_PC (&state->basic, &&rpc_wait_trampoline);
      /* The reply-receiving trampoline code runs initially on the original
	 user stack.  We pass it the signal stack pointer in %r5.  */
      state->basic.r5 = (int) sigsp;
      /* After doing the message receive, the trampoline code will need to
	 update the %r28 value to be restored by sigreturn.  To simplify
	 the assembly code, we pass the address of its slot in SCP to the
	 trampoline code in %r4.  */
      state->basic.r4 = (unsigned int) &scp->sc_regs[27];
      /* Set up the arguments for the handler function in callee-saved
	 registers that we will move to the argument registers after
	 mach_msg_trap returns.  */
      state->basic.r6 = signo;
      state->basic.r7 = sigcode;
      state->basic.r8 = (unsigned int) scp;
    }
  else
    {
      MACHINE_THREAD_STATE_SET_PC (&state->basic, &&trampoline);
      state->basic.r20 = (unsigned int) sigsp;
      /* Set up the arguments for the handler function.  */
      state->basic.r26 = signo;
      state->basic.r25 = sigcode;
      state->basic.r24 = (unsigned int) scp;
    }

  /* We pass the handler function to the trampoline code in %r9.  */
  state->basic.r9 = (unsigned int) handler;
  /* For convenience, we pass the address of __sigreturn in %r10.  */
  state->basic.r10 = (unsigned int) &__sigreturn;
  /* The extra copy of SCP for the __sigreturn arg goes in %r8.  */
  state->basic.r10 = (unsigned int) scp;

  return scp;

  /* The trampoline code follows.  This is not actually executed as part of
     this function, it is just convenient to write it that way.  */

 rpc_wait_trampoline:
  /* This is the entry point when we have an RPC reply message to receive
     before running the handler.  The MACH_MSG_SEND bit has already been
     cleared in the OPTION argument on our stack.  The interrupted user
     stack pointer has not been changed, so the system call can find its
     arguments; the signal stack pointer is in %ebx.  For our convenience,
     %ecx points to the sc_eax member of the sigcontext.  */
  asm volatile
    (/* Retry the interrupted mach_msg system call.  */
     "ldil L%0xC0000000,%r1\nble 4(%sr7,%r1)\n"
     "ldi -25, %r22\n"		/* mach_msg_trap */
     /* When the sigcontext was saved, %r28 was MACH_RCV_INTERRUPTED.  But
	now the message receive has completed and the original caller of
	the RPC (i.e. the code running when the signal arrived) needs to
	see the final return value of the message receive in %r28.  So
	store the new %r28 value into the sc_regs[27] member of the sigcontext
	(whose address is in %r4 to make this code simpler).  */
     "stw (%r4), %r28\n"
     /* Switch to the signal stack.  */
     "copy %r5, %r30\n"
     /* Copy the handler arguments to the argument registers.  */
     "copy %r6, %r26\n"
     "copy %r7, %r25\n"
     "copy %r8, %r24\n"
     );

 trampoline:
  /* Entry point for running the handler normally.  The arguments to the
     handler function are already in the argument registers.  */
  asm volatile
    ("bv (%r9); nop"		/* Call the handler function.  */
     "bv (%r10)\n"		/* Call __sigreturn (SCP); never returns.  */
     "copy %r8, %r26"		/* Set up arg in delay slot.  */
     : : "i" (&__sigreturn));

  /* NOTREACHED */
  return NULL;
}
