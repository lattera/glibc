/* Set thread_state for sighandler, and sigcontext to recover.  MIPS version.
   Copyright (C) 1996, 1997, 1998 Free Software Foundation, Inc.
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
#include <hurd/userlink.h>
#include "thread_state.h"
#include <assert.h>
#include <errno.h>
#include "hurdfault.h"
#include "intr-msg.h"


struct sigcontext *
_hurd_setup_sighandler (struct hurd_sigstate *ss, __sighandler_t handler,
			int signo, struct hurd_signal_detail *detail,
			volatile int rpc_wait,
			struct machine_thread_all_state *state)
{
  __label__ trampoline, rpc_wait_trampoline, firewall;
  void *volatile sigsp;
  struct sigcontext *scp;
  struct 
    {
      int signo;
      long int sigcode;
      struct sigcontext *scp;	/* Points to ctx, below.  */
      void *sigreturn_addr;
      void *sigreturn_returns_here;
      struct sigcontext *return_scp; /* Same; arg to sigreturn.  */
      struct sigcontext ctx;
      struct hurd_userlink link;
    } *stackframe;

  if (ss->context)
    {
      /* We have a previous sigcontext that sigreturn was about
	 to restore when another signal arrived.  We will just base
	 our setup on that.  */
      if (! _hurdsig_catch_memory_fault (ss->context))
	{
	  memcpy (&state->basic, &ss->context->sc_mips_thread_state,
		  sizeof (state->basic));
	  memcpy (&state->exc, &ss->context->sc_mips_exc_state,
		  sizeof (state->exc));
	  state->set = (1 << MIPS_THREAD_STATE) | (1 << MIPS_EXC_STATE);
	  if (state->exc.coproc_state & SC_COPROC_USE_FPU)
	    {
	      memcpy (&state->fpu, &ss->context->sc_mips_float_state,
		      sizeof (state->fpu));
	      state->set |= (1 << MIPS_FLOAT_STATE);
	    }
	}
    }

  if (! machine_get_basic_state (ss->thread, state))
    return NULL;

  /* Save the original SP in the gratuitous s0 ($16) slot.
     We may need to reset the SP (the `r29' slot) to avoid clobbering an
     interrupted RPC frame.  */
  state->basic.r16 = state->basic.r29;

  if ((ss->actions[signo].sa_flags & SA_ONSTACK) &&
      !(ss->sigaltstack.ss_flags & (SS_DISABLE|SS_ONSTACK)))
    {
      sigsp = ss->sigaltstack.ss_sp + ss->sigaltstack.ss_size;
      ss->sigaltstack.ss_flags |= SS_ONSTACK;
      /* XXX need to set up base of new stack for
	 per-thread variables, cthreads.  */
    }
  else
    sigsp = (char *) state->basic.r29;

  /* Push the arguments to call `trampoline' on the stack.  */
  sigsp -= sizeof (*stackframe);
  stackframe = sigsp;

  if (_hurdsig_catch_memory_fault (stackframe))
    {
      /* We got a fault trying to write the stack frame.
	 We cannot set up the signal handler.
	 Returning NULL tells our caller, who will nuke us with a SIGILL.  */
      return NULL;
    }
  else
    {
      int ok;

      extern void _hurdsig_longjmp_from_handler (void *, jmp_buf, int);

      /* Add a link to the thread's active-resources list.  We mark this as
	 the only user of the "resource", so the cleanup function will be
	 called by any longjmp which is unwinding past the signal frame.
	 The cleanup function (in sigunwind.c) will make sure that all the
	 appropriate cleanups done by sigreturn are taken care of.  */
      stackframe->link.cleanup = &_hurdsig_longjmp_from_handler;
      stackframe->link.cleanup_data = &stackframe->ctx;
      stackframe->link.resource.next = NULL;
      stackframe->link.resource.prevp = NULL;
      stackframe->link.thread.next = ss->active_resources;
      stackframe->link.thread.prevp = &ss->active_resources;
      if (stackframe->link.thread.next)
	stackframe->link.thread.next->thread.prevp
	  = &stackframe->link.thread.next;
      ss->active_resources = &stackframe->link;

      /* Set up the arguments for the signal handler.  */
      stackframe->signo = signo;
      stackframe->sigcode = detail->code;
      stackframe->scp = stackframe->return_scp = scp = &stackframe->ctx;
      stackframe->sigreturn_addr = &__sigreturn;
      stackframe->sigreturn_returns_here = &&firewall; /* Crash on return.  */

      /* Set up the sigcontext from the current state of the thread.  */

      scp->sc_onstack = ss->sigaltstack.ss_flags & SS_ONSTACK ? 1 : 0;

      /* struct sigcontext is laid out so that starting at sc_gpr
	 mimics a struct mips_thread_state.  */
      memcpy (&scp->sc_mips_thread_state,
	      &state->basic, sizeof (state->basic));

      /* struct sigcontext is laid out so that starting at sc_cause
	 mimics a struct mips_exc_state.  */
      ok = machine_get_state (ss->thread, state, MIPS_EXC_STATE,
			      &state->exc, &scp->sc_cause,
			      sizeof (state->exc));

      if (ok && (scp->sc_coproc_used & SC_COPROC_USE_FPU))
	/* struct sigcontext is laid out so that starting at sc_fpr
	   mimics a struct mips_float_state.  This state
	   is only meaningful if the coprocessor was used.  */
	  ok = machine_get_state (ss->thread, state, MIPS_FLOAT_STATE,
				  &state->fpu, &scp->sc_mips_float_state,
				  sizeof (state->fpu));

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
	 
	 To do this we change the OPTION argument in its registers to
	 enable only message reception, since the request message has
	 already been sent.  */

      /* The system call arguments are stored in consecutive registers
	 starting with a0 ($4).  */
      struct mach_msg_trap_args *args = (void *) &state->basic.r4;

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

      state->basic.pc = (int) &&rpc_wait_trampoline;
      /* The reply-receiving trampoline code runs initially on the original
	 user stack.  We pass it the signal stack pointer in s4 ($20).  */
      state->basic.r29 = state->basic.r16; /* Restore mach_msg syscall SP.  */
      state->basic.r20 = (int) sigsp;
      /* After doing the message receive, the trampoline code will need to
	 update the v0 ($2) value to be restored by sigreturn.  To simplify
	 the assembly code, we pass the address of its slot in SCP to the
	 trampoline code in s5 ($21).  */
      state->basic.r21 = (int) &scp->sc_gpr[1];
      /* We must preserve the mach_msg_trap args in a0..t2 ($4..$10).
	 Pass the handler args to the trampoline code in s1..s3 ($17..$19).  */
      state->basic.r17 = signo;
      state->basic.r18 = detail->code;
      state->basic.r19 = (int) scp;
    }
  else
    {
      state->basic.pc = (int) &&trampoline;
      state->basic.r29 = (int) sigsp;
      state->basic.r4 = signo;
      state->basic.r5 = detail->code;
      state->basic.r6 = (int) scp;
    }

  /* We pass the handler function to the trampoline code in s6 ($22).  */
  state->basic.r22 = (int) handler;
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
#ifdef __mips64
     "dli $2, -25\n"		/* mach_msg_trap */
#else
     "li $2, -25\n"		/* mach_msg_trap */
#endif
     "syscall\n"
     /* When the sigcontext was saved, v0 was MACH_RCV_INTERRUPTED.  But
	now the message receive has completed and the original caller of
	the RPC (i.e. the code running when the signal arrived) needs to
	see the final return value of the message receive in v0.  So
	store the new v0 value into the sc_gpr[1] member of the sigcontext
	(whose address is in s5 to make this code simpler).  */
#ifdef __mips64
     "sd $2, ($21)\n"
#else
     "sw $2, ($21)\n"
#endif
     /* Since the argument registers needed to have the mach_msg_trap
	arguments, we've stored the arguments to the handler function
	in registers s1..s3 ($17..$19).  */
     "move $4, $17\n"
     "move $5, $18\n"
     "move $6, $19\n"
     /* Switch to the signal stack.  */
     "move $29, $20\n");

 trampoline:
  /* Entry point for running the handler normally.  The arguments to the
     handler function are already in the standard registers:

       a0	SIGNO
       a1	SIGCODE
       a2	SCP
     */
  asm volatile
    ("move $25, $22\n"		/* Copy s6 to t9 for MIPS ABI.  */
     "jal $25; nop\n"		/* Call the handler function.  */
     /* Call __sigreturn (SCP); this cannot return.  */
#ifdef __mips64
     "dla $1,%0\n"
#else
     "la $1,%0\n"
#endif
     "j $1\n"
     "move $4, $16"		/* Set up arg from saved SCP in delay slot.  */
     : : "i" (&__sigreturn));

  /* NOTREACHED */
  asm volatile (".set reorder; .set at; .set macro");

 firewall:
  asm volatile ("hlt: j hlt");

  return NULL;
}
