/* Set thread_state for sighandler, and sigcontext to recover.  i386 version.
Copyright (C) 1994, 1995 Free Software Foundation, Inc.
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
#include <hurd/userlink.h>
#include "thread_state.h"
#include <assert.h>
#include <errno.h>
#include "hurdfault.h"

     
struct mach_msg_trap_args
  {
    void *retaddr;		/* Address mach_msg_trap will return to.  */
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
			int signo, long int sigcode,
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
      if (_hurdsig_catch_fault (SIGSEGV))
	assert (_hurdsig_fault_sigcode >= (long int) ss->context &&
		_hurdsig_fault_sigcode < (long int) (ss->context + 1));
      else
	{
	  memcpy (&state->basic, &ss->context->sc_i386_thread_state,
		  sizeof (state->basic));
	  memcpy (&state->fpu, &ss->context->sc_i386_float_state,
		  sizeof (state->fpu));
	  state->set = (1 << i386_THREAD_STATE) | (1 << i386_FLOAT_STATE);
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
    sigsp = (char *) state->basic.uesp;

  /* Push the arguments to call `trampoline' on the stack.  */
  sigsp -= sizeof (*stackframe);
  stackframe = sigsp;

  if (_hurdsig_catch_fault (SIGSEGV))
    {
      assert (_hurdsig_fault_sigcode >= (long int) stackframe &&
	      _hurdsig_fault_sigcode <= (long int) (stackframe + 1));
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
      stackframe->sigcode = sigcode;
      stackframe->scp = stackframe->return_scp = scp = &stackframe->ctx;
      stackframe->sigreturn_addr = &__sigreturn;
      stackframe->sigreturn_returns_here = &&firewall; /* Crash on return.  */

      /* Set up the sigcontext from the current state of the thread.  */

      scp->sc_onstack = ss->sigaltstack.ss_flags & SA_ONSTACK ? 1 : 0;

      /* struct sigcontext is laid out so that starting at sc_gs mimics a
	 struct i386_thread_state.  */
      memcpy (&scp->sc_i386_thread_state,
	      &state->basic, sizeof (state->basic));

      /* struct sigcontext is laid out so that starting at sc_fpkind mimics
	 a struct i386_float_state.  */
      ok = machine_get_state (ss->thread, state, i386_FLOAT_STATE,
			      &state->fpu, &scp->sc_i386_float_state,
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
	 
	 To do this we change the OPTION argument on its stack to enable only
	 message reception, since the request message has already been
	 sent.  */

      struct mach_msg_trap_args *args = (void *) state->basic.uesp;

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

      _hurdsig_end_catch_fault ();

      state->basic.eip = (int) &&rpc_wait_trampoline;
      /* The reply-receiving trampoline code runs initially on the original
	 user stack.  We pass it the signal stack pointer in %ebx.  */
      state->basic.ebx = (int) sigsp;
      /* After doing the message receive, the trampoline code will need to
	 update the %eax value to be restored by sigreturn.  To simplify
	 the assembly code, we pass the address of its slot in SCP to the
	 trampoline code in %ecx.  */
      state->basic.ecx = (int) &scp->sc_eax;
    }
  else
    {
      state->basic.eip = (int) &&trampoline;
      state->basic.uesp = (int) sigsp;
    }
  /* We pass the handler function to the trampoline code in %edx.  */
  state->basic.edx = (int) handler;

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
     "movl $-25, %eax\n"	/* mach_msg_trap */
     "lcall $7, $0\n"
     /* When the sigcontext was saved, %eax was MACH_RCV_INTERRUPTED.  But
	now the message receive has completed and the original caller of
	the RPC (i.e. the code running when the signal arrived) needs to
	see the final return value of the message receive in %eax.  So
	store the new %eax value into the sc_eax member of the sigcontext
	(whose address is in %ecx to make this code simpler).  */
     "movl %eax, (%ecx)\n"
     /* Switch to the signal stack.  */
     "movl %ebx, %esp\n");

 trampoline:
  /* Entry point for running the handler normally.  The arguments to the
     handler function are already on the top of the stack:

       0(%esp)	SIGNO
       4(%esp)	SIGCODE
       8(%esp)	SCP
     */
  asm volatile
    ("call *%edx\n"		/* Call the handler function.  */
     "addl $12, %esp\n"		/* Pop its args.  */
     /* The word at the top of stack is &__sigreturn; following are a dummy
	word to fill the slot for the address for __sigreturn to return to,
	and a copy of SCP for __sigreturn's argument.  "Return" to calling
	__sigreturn (SCP); this call never returns.  */
     "ret");

 firewall:
  asm volatile ("hlt");

  /* NOTREACHED */
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
  static const unsigned char syscall[] = { 0x9a, 0, 0, 0, 0, 7, 0 };
  const unsigned char *volatile pc
    = (void *) state->basic.eip - sizeof syscall;

  if (_hurdsig_catch_fault (SIGSEGV))
    assert (_hurdsig_fault_sigcode >= (long int) pc &&
	    _hurdsig_fault_sigcode < (long int) (pc + sizeof syscall));
  else
    {
      int rcving = (state->basic.eax == MACH_RCV_INTERRUPTED &&
		    !memcmp (pc, &syscall, sizeof syscall));
      _hurdsig_end_catch_fault ();
      if (rcving)
	{
	  /* We did just return from a mach_msg_trap system call
	     doing a message receive that was interrupted.
	     Examine the parameters to find the receive right.  */
	  struct mach_msg_trap_args *args = (void *) state->basic.uesp;

	  *port = args->rcv_name;
	  return 1;
	}
    }

  return 0;
}
