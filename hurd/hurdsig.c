/* Copyright (C) 1991, 1992, 1993, 1994, 1995 Free Software Foundation, Inc.
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

#include <stdlib.h>
#include <stdio.h>
#include <hurd.h>
#include <hurd/signal.h>
#include <cthreads.h>		/* For `struct mutex'.  */
#include <string.h>
#include "hurdfault.h"
#include "hurdmalloc.h"		/* XXX */

const char *_hurdsig_getenv (const char *);

struct mutex _hurd_siglock;
int _hurd_stopped;

/* Port that receives signals and other miscellaneous messages.  */
mach_port_t _hurd_msgport;

/* Thread listening on it.  */
thread_t _hurd_msgport_thread;

/* Thread which receives task-global signals.  */
thread_t _hurd_sigthread;

/* Linked-list of per-thread signal state.  */
struct hurd_sigstate *_hurd_sigstates;

static void
default_sigaction (struct sigaction actions[NSIG])
{
  int signo;

  __sigemptyset (&actions[0].sa_mask);
  actions[0].sa_flags = SA_RESTART;
  actions[0].sa_handler = SIG_DFL;

  for (signo = 1; signo < NSIG; ++signo)
    actions[signo] = actions[0];
}

struct hurd_sigstate *
_hurd_thread_sigstate (thread_t thread)
{
  struct hurd_sigstate *ss;
  __mutex_lock (&_hurd_siglock);
  for (ss = _hurd_sigstates; ss != NULL; ss = ss->next)
    if (ss->thread == thread)
      break;
  if (ss == NULL)
    {
      ss = malloc (sizeof (*ss));
      if (ss == NULL)
	__libc_fatal ("hurd: Can't allocate thread sigstate\n");
      ss->thread = thread;
      __spin_lock_init (&ss->lock);

      /* Initialze default state.  */
      __sigemptyset (&ss->blocked);
      __sigemptyset (&ss->pending);
      memset (&ss->sigaltstack, 0, sizeof (ss->sigaltstack));
      ss->suspended = 0;
#ifdef noteven
      __condition_init (&ss->arrived);
#endif
      ss->intr_port = MACH_PORT_NULL;
      ss->context = NULL;

      /* Initialize the sigaction vector from the default signal receiving
	 thread's state, and its from the system defaults.  */
      if (thread == _hurd_sigthread)
	default_sigaction (ss->actions);
      else
	{
	  struct hurd_sigstate *s;
	  for (s = _hurd_sigstates; s != NULL; s = s->next)
	    if (s->thread == _hurd_sigthread)
	      break;
	  if (s)
	    {
	      __spin_lock (&s->lock);
	      memcpy (ss->actions, s->actions, sizeof (s->actions));
	      __spin_unlock (&s->lock);
	    }
	  else
	    default_sigaction (ss->actions);
	}

      ss->next = _hurd_sigstates;
      _hurd_sigstates = ss;
    }
  __mutex_unlock (&_hurd_siglock);
  return ss;
}

/* Signal delivery itself is on this page.  */

#include <hurd/fd.h>
#include <hurd/core.h>
#include <hurd/paths.h>
#include <setjmp.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "thread_state.h"
#include <hurd/msg_server.h>
#include <hurd/msg_reply.h>	/* For __msg_sig_post_reply.  */
#include <assert.h>
#include <hurd/interrupt.h>

int _hurd_core_limit;	/* XXX */

/* Call the core server to mummify us before we die.
   Returns nonzero if a core file was written.  */
static int
write_corefile (int signo, long int sigcode, int sigerror)
{
  error_t err;
  mach_port_t coreserver;
  file_t file, coredir;
  const char *name;

  /* XXX RLIMIT_CORE:
     When we have a protocol to make the server return an error
     for RLIMIT_FSIZE, then tell the corefile fs server the RLIMIT_CORE
     value in place of the RLIMIT_FSIZE value.  */

  /* First get a port to the core dumping server.  */
  coreserver = MACH_PORT_NULL;
  name = _hurdsig_getenv ("CORESERVER");
  if (name != NULL)
    coreserver = __file_name_lookup (name, 0, 0);
  if (coreserver == MACH_PORT_NULL)
    coreserver = __file_name_lookup (_SERVERS_CORE, 0, 0);
  if (coreserver == MACH_PORT_NULL)
    return 0;

  /* Get a port to the directory where the new core file will reside.  */
  name = _hurdsig_getenv ("COREFILE");
  if (name == NULL)
    name = "core";
  coredir = __file_name_split (name, (char **) &name);
  if (coredir == MACH_PORT_NULL)
    return 0;
  /* Create the new file, but don't link it into the directory yet.  */
  if (err = __dir_mkfile (coredir, O_WRONLY|O_CREAT,
			  0600 & ~_hurd_umask, /* XXX ? */
			  &file))
    return 0;

  /* Call the core dumping server to write the core file.  */
  err = __core_dump_task (coreserver,
			  __mach_task_self (),
			  file, _hurdsig_getenv ("GNUTARGET"),
			  signo, sigcode, sigerror);
  __mach_port_deallocate (__mach_task_self (), coreserver);
  if (! err)
    /* The core dump into FILE succeeded, so now link it into the
       directory.  */
    err = __dir_link (file, coredir, name);
  __mach_port_deallocate (__mach_task_self (), file);
  __mach_port_deallocate (__mach_task_self (), coredir);
  return !err;
}


/* Send a sig_post reply message if it hasn't already been sent.  */
static inline void
post_reply (mach_port_t *reply_port, mach_msg_type_name_t reply_port_type,
	    int untraced,
	    error_t result)
{
  if (reply_port == NULL || *reply_port == MACH_PORT_NULL)
    return;
  (untraced ? __msg_sig_post_untraced_reply : __msg_sig_post_reply)
    (*reply_port, reply_port_type, result);
  *reply_port = MACH_PORT_NULL;
}


/* The lowest-numbered thread state flavor value is 1,
   so we use bit 0 in machine_thread_all_state.set to
   record whether we have done thread_abort.  */
#define THREAD_ABORTED 1

/* SS->thread is suspended.  Abort the thread and get its basic state.  If
   REPLY_PORT is not NULL, send a reply on *REPLY_PORT after aborting the
   thread.  */
static void
abort_thread (struct hurd_sigstate *ss, struct machine_thread_all_state *state,
	      mach_port_t *reply_port, mach_msg_type_name_t reply_port_type,
	      int untraced)
{
  if (!(state->set & THREAD_ABORTED))
    {
      __thread_abort (ss->thread);
      /* Clear all thread state flavor set bits, because thread_abort may
	 have changed the state.  */
      state->set = THREAD_ABORTED;
    }

  if (reply_port)
    post_reply (reply_port, reply_port_type, untraced, 0);

  machine_get_basic_state (ss->thread, state);
}

/* Find the location of the MiG reply port cell in use by the thread whose
   state is described by THREAD_STATE.  Make sure that this location can be
   set without faulting, or else return NULL.  */

static mach_port_t *
interrupted_reply_port_location (struct machine_thread_all_state *thread_state)
{
  mach_port_t *portloc = (mach_port_t *) __hurd_threadvar_location_from_sp
    (_HURD_THREADVAR_MIG_REPLY, (void *) thread_state->basic.SP);

  if (_hurdsig_catch_fault (SIGSEGV))
    {
      assert (_hurdsig_fault_sigcode == (long int) portloc);
      /* Faulted trying to read the stack.  */
      return NULL;
    }

  /* Fault now if this pointer is bogus.  */
  *(volatile mach_port_t *) portloc = *portloc;

  _hurdsig_end_catch_fault ();

  return portloc;
}


/* SS->thread is suspended.

   Abort any interruptible RPC operation the thread is doing.

   This uses only the constant member SS->thread and the unlocked, atomically
   set member SS->intr_port, so no locking is needed.

   If successfully sent an interrupt_operation and therefore the thread should
   wait for its pending RPC to return (possibly EINTR) before taking the
   incoming signal, returns the reply port to be received on.  Otherwise
   returns MACH_PORT_NULL.

   *STATE_CHANGE is set nonzero if STATE->basic was modified and should
   be applied back to the thread if it might ever run again, else zero.  */

static mach_port_t
abort_rpcs (struct hurd_sigstate *ss, int signo,
	    struct machine_thread_all_state *state, int *state_change,
	    mach_port_t *reply_port, mach_msg_type_name_t reply_port_type,
	    int untraced)
{
  mach_port_t msging_port;
  mach_port_t intr_port;

  *state_change = 0;

  intr_port = ss->intr_port;
  if (intr_port == MACH_PORT_NULL)
    /* No interruption needs done.  */
    return MACH_PORT_NULL;

  /* Abort the thread's kernel context, so any pending message send or
     receive completes immediately or aborts.  */
  abort_thread (ss, state, reply_port, reply_port_type, untraced);

  if (_hurdsig_rcv_interrupted_p (state, &msging_port))
    {
      error_t err;

      /* The RPC request message was sent and the thread was waiting for
	 the reply message; now the message receive has been aborted, so
	 the mach_msg_call will return MACH_RCV_INTERRUPTED.  We must tell
	 the server to interrupt the pending operation.  The thread must
	 wait for the reply message before running the signal handler (to
	 guarantee that the operation has finished being interrupted), so
	 our nonzero return tells the trampoline code to finish the message
	 receive operation before running the handler.  */

      err = __interrupt_operation (intr_port);

      if (err)
	{
	  mach_port_t *reply;

	  /* The interrupt didn't work.
	     Destroy the receive right the thread is blocked on.  */
	  __mach_port_destroy (__mach_task_self (), msging_port);

	  /* The system call return value register now contains
	     MACH_RCV_INTERRUPTED; when mach_msg resumes, it will retry the
	     call.  Since we have just destroyed the receive right, the
	     retry will fail with MACH_RCV_INVALID_NAME.  Instead, just
	     change the return value here to EINTR so mach_msg will not
	     retry and the EINTR error code will propagate up.  */
	  state->basic.SYSRETURN = EINTR;
	  *state_change = 1;

	  /* If that was the thread's MiG reply port (which I think should
	     always be the case), clear the reply port cell so it won't be
	     reused.  */
	  reply = interrupted_reply_port_location (state);
	  if (reply != NULL && *reply == msging_port)
	    *reply = MACH_PORT_NULL;
	}

      /* All threads whose RPCs were interrupted by the interrupt_operation
	 call above will retry their RPCs unless we clear SS->intr_port.
	 So we clear it for the thread taking a signal when SA_RESTART is
	 clear, so that its call returns EINTR.  */
      if (!(ss->actions[signo].sa_flags & SA_RESTART))
	ss->intr_port = MACH_PORT_NULL;

      return err ? MACH_PORT_NULL : msging_port;
    }

  /* One of the following is true:

     1. The RPC has not yet been sent.  The thread will start its operation
     after the signal has been handled.

     2. The RPC has finished, but not yet cleared SS->intr_port.
     The thread will clear SS->intr_port after running the handler.

     3. The RPC request message was being sent was aborted.  The mach_msg
     system call will return MACH_SEND_INTERRUPTED, and HURD_EINTR_RPC will
     notice the interruption (either retrying the RPC or returning EINTR).  */

  return MACH_PORT_NULL;
}

/* Abort the RPCs being run by all threads but this one;
   all other threads should be suspended.  If LIVE is nonzero, those
   threads may run again, so they should be adjusted as necessary to be
   happy when resumed.  STATE is clobbered as a scratch area; its initial
   contents are ignored, and its contents on return are not useful.  */

static void
abort_all_rpcs (int signo, struct machine_thread_all_state *state, int live)
{
  /* We can just loop over the sigstates.  Any thread doing something
     interruptible must have one.  We needn't bother locking because all
     other threads are stopped.  */

  struct hurd_sigstate *ss;
  size_t nthreads;
  mach_port_t *reply_ports;

  /* First loop over the sigstates to count them.
     We need to know how big a vector we will need for REPLY_PORTS.  */
  nthreads = 0;
  for (ss = _hurd_sigstates; ss != NULL; ss = ss->next)
    ++nthreads;

  reply_ports = alloca (nthreads * sizeof *reply_ports);

  nthreads = 0;
  for (ss = _hurd_sigstates; ss != NULL; ss = ss->next)
    if (ss->thread == _hurd_msgport_thread)
      reply_ports[nthreads++] = MACH_PORT_NULL;
    else
      {
	int state_changed;
	state->set = 0;		/* Reset scratch area.  */

	/* Abort any operation in progress with interrupt_operation.
	   Record the reply port the thread is waiting on.
	   We will wait for all the replies below.  */
	reply_ports[nthreads++] = abort_rpcs (ss, signo, state, &state_changed,
					      NULL, 0, 0);
	if (state_changed && live)
	  /* Aborting the RPC needed to change this thread's state,
	     and it might ever run again.  So write back its state.  */
	  __thread_set_state (ss->thread, MACHINE_THREAD_STATE_FLAVOR,
			      (natural_t *) &state->basic,
			      MACHINE_THREAD_STATE_COUNT);
      }

  /* Wait for replies from all the successfully interrupted RPCs.  */
  while (nthreads-- > 0)
    if (reply_ports[nthreads] != MACH_PORT_NULL)
      {
	error_t err;
	mach_msg_header_t head;
	err = __mach_msg (&head, MACH_RCV_MSG, 0, sizeof head,
			  reply_ports[nthreads],
			  MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
	if (err != MACH_RCV_TOO_LARGE)
	  assert_perror (err);
      }
}


struct hurd_signal_preempt *_hurd_signal_preempt[NSIG];
struct mutex _hurd_signal_preempt_lock;

/* Mask of stop signals.  */
#define STOPSIGS (sigmask (SIGTTIN) | sigmask (SIGTTOU) | \
		  sigmask (SIGSTOP) | sigmask (SIGTSTP))

/* Deliver a signal.  SS is not locked.  */
void
_hurd_internal_post_signal (struct hurd_sigstate *ss,
			    int signo, long int sigcode, int sigerror,
			    mach_port_t reply_port,
			    mach_msg_type_name_t reply_port_type,
			    int untraced)
{
  struct machine_thread_all_state thread_state;
  enum { stop, ignore, core, term, handle } act;
  sighandler_t handler;
  struct hurd_signal_preempt *pe;
  sighandler_t (*preempt) (thread_t, int, long int, int) = NULL;
  sigset_t pending;
  int ss_suspended;

  /* Reply to this sig_post message.  */
  inline void reply (void)
    {
      post_reply (&reply_port, reply_port_type, untraced, 0);
    }

  /* Mark the signal as pending.  */
  void mark_pending (void)
    {
      __sigaddset (&ss->pending, signo);
      /* Save the code to be given to the handler when SIGNO is
	 unblocked.  */
      ss->pending_data[signo].code = sigcode;
      ss->pending_data[signo].error = sigerror;
    }

  /* Suspend the process with SIGNO.  */
  void suspend (void)
    {
      /* Stop all other threads and mark ourselves stopped.  */
      __USEPORT (PROC,
		 ({
		   /* Hold the siglock while stopping other threads to be
		      sure it is not held by another thread afterwards.  */
		   __mutex_lock (&_hurd_siglock);
		   __proc_dostop (port, _hurd_msgport_thread);
		   __mutex_unlock (&_hurd_siglock);
		   abort_all_rpcs (signo, &thread_state, 1);
		   __proc_mark_stop (port, signo);
		 }));
      _hurd_stopped = 1;
    }

 post_signal:

  thread_state.set = 0;		/* We know nothing.  */

  /* Check for a preempted signal.  Preempted signals
     can arrive during critical sections.  */
  __mutex_lock (&_hurd_signal_preempt_lock);
  for (pe = _hurd_signal_preempt[signo]; pe != NULL; pe = pe->next)
    if (pe->handler && sigcode >= pe->first && sigcode <= pe->last)
      {
	preempt = pe->handler;
	break;
      }
  __mutex_unlock (&_hurd_signal_preempt_lock);

  handler = SIG_DFL;
  if (preempt)
    /* Let the preempting handler examine the thread.
       If it returns SIG_DFL, we run the normal handler;
       otherwise we use the handler it returns.  */
    handler = (*preempt) (ss->thread, signo, sigcode, sigerror);

  ss_suspended = 0;

  if (handler != SIG_DFL)
    /* Run the preemption-provided handler.  */
    act = handle;
  else
    {
      /* No preemption.  Do normal handling.  */

      __spin_lock (&ss->lock);

      handler = ss->actions[signo].sa_handler;

      if (!untraced && (_hurd_exec_flags & EXEC_TRACED))
	{
	  /* We are being traced.  Stop to tell the debugger of the signal.  */
	  if (_hurd_stopped)
	    /* Already stopped.  Mark the signal as pending;
	       when resumed, we will notice it and stop again.  */
	    mark_pending ();
	  else
	    suspend ();
	  __spin_unlock (&ss->lock);
	  reply ();
	  return;
	}

      if (handler == SIG_DFL)
	/* Figure out the default action for this signal.  */
	switch (signo)
	  {
	  case 0:
	    /* A sig_post msg with SIGNO==0 is sent to
	       tell us to check for pending signals.  */
	    act = ignore;
	    break;

	  case SIGTTIN:
	  case SIGTTOU:
	  case SIGSTOP:
	  case SIGTSTP:
	    act = stop;
	    break;

	  case SIGCONT:
	  case SIGIO:
	  case SIGURG:
	  case SIGCHLD:
	  case SIGWINCH:
	    act = ignore;
	    break;

	  case SIGQUIT:
	  case SIGILL:
	  case SIGTRAP:
	  case SIGIOT:
	  case SIGEMT:
	  case SIGFPE:
	  case SIGBUS:
	  case SIGSEGV:
	  case SIGSYS:
	    act = core;
	    break;

	  case SIGINFO:
	    if (_hurd_pgrp == _hurd_pid)
	      {
		/* We are the process group leader.  Since there is no
		   user-specified handler for SIGINFO, we use a default one
		   which prints something interesting.  We use the normal
		   handler mechanism instead of just doing it here to avoid
		   the signal thread faulting or blocking in this
		   potentially hairy operation.  */
		act = handle;
		handler = _hurd_siginfo_handler;
	      }
	    else
	      act = ignore;
	    break;

	  default:
	    act = term;
	    break;
	  }
      else if (handler == SIG_IGN)
	act = ignore;
      else
	act = handle;

      if (__sigmask (signo) & STOPSIGS)
	/* Stop signals clear a pending SIGCONT even if they
	   are handled or ignored (but not if preempted).  */
	ss->pending &= ~sigmask (SIGCONT);
      else
	{
	  if (signo == SIGCONT)
	    /* Even if handled or ignored (but not preempted), SIGCONT clears
	       stop signals and resumes the process.  */
	    ss->pending &= ~STOPSIGS;

	  if (_hurd_stopped && act != stop && (untraced || signo == SIGCONT))
	    {
	      /* Resume the process from being stopped.  */
	      thread_t *threads;
	      mach_msg_type_number_t nthreads, i;
	      error_t err;
	      /* Tell the proc server we are continuing.  */
	      __USEPORT (PROC, __proc_mark_cont (port));
	      /* Fetch ports to all our threads and resume them.  */
	      err = __task_threads (__mach_task_self (), &threads, &nthreads);
	      assert_perror (err);
	      for (i = 0; i < nthreads; ++i)
		{
		  if (threads[i] != _hurd_msgport_thread &&
		      (act != handle || threads[i] != ss->thread))
		    __thread_resume (threads[i]);
		  __mach_port_deallocate (__mach_task_self (), threads[i]);
		}
	      __vm_deallocate (__mach_task_self (),
			       (vm_address_t) threads,
			       nthreads * sizeof *threads);
	      _hurd_stopped = 0;
	      /* The thread that will run the handler is already suspended.  */
	      ss_suspended = 1;
	    }
	}
    }

  if (_hurd_orphaned && act == stop &&
      (__sigmask (signo) & (__sigmask (SIGTTIN) | __sigmask (SIGTTOU) |
			    __sigmask (SIGTSTP))))
    {
      /* If we would ordinarily stop for a job control signal, but we are
	 orphaned so noone would ever notice and continue us again, we just
	 quietly die, alone and in the dark.  */
      sigcode = signo;
      signo = SIGKILL;
      act = term;
    }

  /* Handle receipt of a blocked signal, or any signal while stopped.
     It matters that we test ACT first here, because we must never pass
     SIGNO==0 to __sigismember.  */
  if ((act != ignore && __sigismember (&ss->blocked, signo)) ||
      (signo != SIGKILL && _hurd_stopped))
    {
      mark_pending ();
      act = ignore;
    }

  /* Perform the chosen action for the signal.  */
  switch (act)
    {
    case stop:
      if (_hurd_stopped)
	/* We are already stopped, but receiving an untraced stop
	   signal.  Instead of resuming and suspending again, just
	   notify the proc server of the new stop signal.  */
	__USEPORT (PROC, __proc_mark_stop (port, signo));
      else
	/* Suspend the process.  */
	suspend ();
      break;

    case ignore:
      /* Nobody cares about this signal.  */
      break;

    case term:			/* Time to die.  */
    case core:			/* And leave a rotting corpse.  */
    nirvana:
      /* Have the proc server stop all other threads in our task.  */
      __USEPORT (PROC, __proc_dostop (port, _hurd_msgport_thread));
      /* No more user instructions will be executed.
	 The signal can now be considered delivered.  */
      reply ();
      /* Abort all server operations now in progress.  */
      abort_all_rpcs (signo, &thread_state, 0);

      {
	int status = W_EXITCODE (0, signo);
	/* Do a core dump if desired.  Only set the wait status bit saying we
	   in fact dumped core if the operation was actually successful.  */
	if (act == core && write_corefile (signo, sigcode, sigerror))
	  status |= WCOREFLAG;
	/* Tell proc how we died and then stick the saber in the gut.  */
	_hurd_exit (status);
	/* NOTREACHED */
      }

    case handle:
      /* Call a handler for this signal.  */
      {
	struct sigcontext *scp;
	int wait_for_reply, state_changed;

	/* Stop the thread and abort its pending RPC operations.  */
	if (! ss_suspended)
	  __thread_suspend (ss->thread);

	/* Abort the thread's kernel context, so any pending message send
	   or receive completes immediately or aborts.  If an interruptible
	   RPC is in progress, abort_rpcs will do this.  But we must always
	   do it before fetching the thread's state, because
	   thread_get_state is never kosher before thread_abort.  */
	abort_thread (ss, &thread_state, NULL, 0, 0);

	wait_for_reply = (abort_rpcs (ss, signo, &thread_state, &state_changed,
				      &reply_port, reply_port_type, untraced)
			  != MACH_PORT_NULL);

	if (ss->critical_section)
	  {
	    /* The thread is in a critical section.  Mark the signal as
	       pending.  When it finishes the critical section, it will
	       check for pending signals.  */
	    mark_pending ();
	    assert (! state_changed);
	    __thread_resume (ss->thread);
	    break;
	  }

	/* Call the machine-dependent function to set the thread up
	   to run the signal handler, and preserve its old context.  */
	scp = _hurd_setup_sighandler (ss, handler,
				      signo, sigcode,
				      wait_for_reply, &thread_state);
	if (scp == NULL)
	  {
	    /* We got a fault setting up the stack frame for the handler.
	       Nothing to do but die; BSD gets SIGILL in this case.  */
	    sigcode = signo;	/* XXX ? */
	    signo = SIGILL;
	    act = core;
	    goto nirvana;
	  }

	/* Set the machine-independent parts of the signal context.  */

	scp->sc_error = sigerror;
	{
	  /* Fetch the thread variable for the MiG reply port,
	     and set it to MACH_PORT_NULL.  */
	  mach_port_t *loc = interrupted_reply_port_location (&thread_state);
	  if (loc)
	    {
	      scp->sc_reply_port = *loc;
	      *loc = MACH_PORT_NULL;
	    }
	  else
	    scp->sc_reply_port = MACH_PORT_NULL;
	}

	/* Block SIGNO and requested signals while running the handler.  */
	scp->sc_mask = ss->blocked;
	ss->blocked |= __sigmask (signo) | ss->actions[signo].sa_mask;

	/* Save the intr_port in use by the interrupted code,
	   and clear the cell before running the trampoline.  */
	scp->sc_intr_port = ss->intr_port;
	ss->intr_port = MACH_PORT_NULL;

	/* Start the thread running the handler (or possibly waiting for an
	   RPC reply before running the handler).  */
	__thread_set_state (ss->thread, MACHINE_THREAD_STATE_FLAVOR,
			    (natural_t *) &thread_state.basic,
			    MACHINE_THREAD_STATE_COUNT);
	__thread_resume (ss->thread);
	thread_state.set = 0;	/* Everything we know is now wrong.  */
	break;
      }
    }

  /* The signal has either been ignored or is now being handled.  We can
     consider it delivered and reply to the killer.  The exception is
     signal 0, which can be sent by a user thread to make us check for
     pending signals.  In that case we want to deliver the pending signals
     before replying.  */
  if (signo != 0)
    reply ();

  /* We get here unless the signal was fatal.  We still hold SS->lock.
     Check for pending signals, and loop to post them.  */
#define PENDING	(!_hurd_stopped && (pending = ss->pending & ~ss->blocked))
  if (PENDING)
    {
    pending:
      for (signo = 1; signo < NSIG; ++signo)
	if (__sigismember (&pending, signo))
	  {
	    __sigdelset (&ss->pending, signo);
	    sigcode = ss->pending_data[signo].code;
	    sigerror = ss->pending_data[signo].error;
	    __spin_unlock (&ss->lock);
	    goto post_signal;
	  }
    }

  /* No pending signals left undelivered for this thread.
     If we were sent signal 0, we need to check for pending
     signals for all threads.  */
  if (signo == 0)
    {
      __spin_unlock (&ss->lock);
      __mutex_lock (&_hurd_siglock);
      for (ss = _hurd_sigstates; ss != NULL; ss = ss->next)
	{
	  __spin_lock (&ss->lock);
	  if (PENDING)
	    goto pending;
	  __spin_unlock (&ss->lock);
	}
      __mutex_unlock (&_hurd_siglock);
    }
  else
    {
      /* No more signals pending; SS->lock is still locked.
	 Wake up any sigsuspend call that is blocking SS->thread.  */
      if (ss->suspended != MACH_PORT_NULL)
	{
	  /* There is a sigsuspend waiting.  Tell it to wake up.  */
	  error_t err;
	  mach_msg_header_t msg;
	  err = __mach_port_insert_right (__mach_task_self (),
					  ss->suspended, ss->suspended,
					  MACH_MSG_TYPE_MAKE_SEND);
	  assert_perror (err);
	  msg.msgh_bits = MACH_MSGH_BITS (MACH_MSG_TYPE_MOVE_SEND, 0);
	  msg.msgh_remote_port = ss->suspended;
	  msg.msgh_local_port = MACH_PORT_NULL;
	  /* These values do not matter.  */
	  msg.msgh_id = 8675309; /* Jenny, Jenny.  */
	  msg.msgh_seqno = 17;	/* Random.  */
	  ss->suspended = MACH_PORT_NULL;
	  err = __mach_msg (&msg, MACH_SEND_MSG, sizeof msg, 0,
			    MACH_PORT_NULL, MACH_MSG_TIMEOUT_NONE,
			    MACH_PORT_NULL);
	  assert_perror (err);
	}
      __spin_unlock (&ss->lock);
    }

  /* All pending signals delivered to all threads.
     Now we can send the reply message even for signal 0.  */
  reply ();
}

/* Decide whether REFPORT enables the sender to send us a SIGNO signal.
   Returns zero if so, otherwise the error code to return to the sender.  */

static error_t
signal_allowed (int signo, mach_port_t refport)
{
  if (signo < 0 || signo >= NSIG)
    return EINVAL;

  if (refport == __mach_task_self ())
    /* Can send any signal.  */
    goto win;

  /* Avoid needing to check for this below.  */
  if (refport == MACH_PORT_NULL)
    return EPERM;

  switch (signo)
    {
    case SIGINT:
    case SIGQUIT:
    case SIGTSTP:
    case SIGHUP:
    case SIGINFO:
    case SIGTTIN:
    case SIGTTOU:
      /* Job control signals can be sent by the controlling terminal.  */
      if (__USEPORT (CTTYID, port == refport))
	goto win;
      break;

    case SIGCONT:
      {
	/* A continue signal can be sent by anyone in the session.  */
	mach_port_t sessport;
	if (! __USEPORT (PROC, __proc_getsidport (port, &sessport)))
	  { 
	    __mach_port_deallocate (__mach_task_self (), sessport);
	    if (refport == sessport)
	      goto win;
	  }
      }
      break;

    case SIGIO:
    case SIGURG:
      {
	/* Any io object a file descriptor refers to might send us
	   one of these signals using its async ID port for REFPORT.

	   This is pretty wide open; it is not unlikely that some random
	   process can at least open for reading something we have open,
	   get its async ID port, and send us a spurious SIGIO or SIGURG
	   signal.  But BSD is actually wider open than that!--you can set
	   the owner of an io object to any process or process group
	   whatsoever and send them gratuitous signals.

	   Someday we could implement some reasonable scheme for
	   authorizing SIGIO and SIGURG signals properly.  */

	int d;
	__mutex_lock (&_hurd_dtable_lock);
	for (d = 0; (unsigned int) d < (unsigned int) _hurd_dtablesize; ++d)
	  {
	    struct hurd_userlink ulink;
	    io_t port;
	    mach_port_t asyncid;
	    if (_hurd_dtable[d] == NULL)
	      continue;
	    port = _hurd_port_get (&_hurd_dtable[d]->port, &ulink);
	    if (! __io_get_icky_async_id (port, &asyncid))
	      {
		if (refport == asyncid)
		  /* Break out of the loop on the next iteration.  */
		  d = -1;
		__mach_port_deallocate (__mach_task_self (), asyncid);
	      }
	    _hurd_port_free (&_hurd_dtable[d]->port, &ulink, port);
	  }
	/* If we found a lucky winner, we've set D to -1 in the loop.  */
	if (d < 0)
	  goto win;
      }
    }

  /* If this signal is legit, we have done `goto win' by now.
     When we return the error, mig deallocates REFPORT.  */
  return EPERM;

 win:
  /* Deallocate the REFPORT send right; we are done with it.  */
  __mach_port_deallocate (__mach_task_self (), refport);

  return 0;
}

/* Implement the sig_post RPC from <hurd/msg.defs>;
   sent when someone wants us to get a signal.  */
kern_return_t
_S_msg_sig_post (mach_port_t me,
		 mach_port_t reply_port, mach_msg_type_name_t reply_port_type,
		 int signo,
		 mach_port_t refport)
{
  error_t err;

  if (err = signal_allowed (signo, refport))
    return err;

  /* Post the signal to the designated signal-receiving thread.  This will
     reply when the signal can be considered delivered.  */
  _hurd_internal_post_signal (_hurd_thread_sigstate (_hurd_sigthread),
			      signo, 0, 0, reply_port, reply_port_type,
			      0); /* Stop if traced.  */

  return MIG_NO_REPLY;		/* Already replied.  */
}

/* Implement the sig_post_untraced RPC from <hurd/msg.defs>;
   sent when the debugger wants us to really get a signal
   even if we are traced.  */
kern_return_t
_S_msg_sig_post_untraced (mach_port_t me,
			  mach_port_t reply_port,
			  mach_msg_type_name_t reply_port_type,
			  int signo,
			  mach_port_t refport)
{
  error_t err;

  if (err = signal_allowed (signo, refport))
    return err;

  /* Post the signal to the designated signal-receiving thread.  This will
     reply when the signal can be considered delivered.  */
  _hurd_internal_post_signal (_hurd_thread_sigstate (_hurd_sigthread),
			      signo, 0, 0, reply_port, reply_port_type,
			      1); /* Untraced flag. */

  return MIG_NO_REPLY;		/* Already replied.  */
}

extern void __mig_init (void *);

#include <mach/task_special_ports.h>

/* Initialize the message port and _hurd_sigthread and start the signal
   thread.  */

void
_hurdsig_init (void)
{
  error_t err;
  vm_size_t stacksize;

  __mutex_init (&_hurd_siglock);

  if (err = __mach_port_allocate (__mach_task_self (),
				  MACH_PORT_RIGHT_RECEIVE,
				  &_hurd_msgport))
    __libc_fatal ("hurd: Can't create message port receive right\n");
  
  /* Make a send right to the signal port.  */
  if (err = __mach_port_insert_right (__mach_task_self (),
				      _hurd_msgport,
				      _hurd_msgport,
				      MACH_MSG_TYPE_MAKE_SEND))
    __libc_fatal ("hurd: Can't create send right to message port\n");

  /* Set the default thread to receive task-global signals
     to this one, the main (first) user thread.  */
  _hurd_sigthread = __mach_thread_self ();

  /* Start the signal thread listening on the message port.  */

  if (err = __thread_create (__mach_task_self (), &_hurd_msgport_thread))
    __libc_fatal ("hurd: Can't create signal thread\n");

  stacksize = __vm_page_size * 4; /* Small stack for signal thread.  */
  if (err = __mach_setup_thread (__mach_task_self (), _hurd_msgport_thread,
				 _hurd_msgport_receive,
				 (vm_address_t *) &__hurd_sigthread_stack_base,
				 &stacksize))
    __libc_fatal ("hurd: Can't setup signal thread\n");

  __hurd_sigthread_stack_end = __hurd_sigthread_stack_base + stacksize;
  __hurd_sigthread_variables =
    malloc (__hurd_threadvar_max * sizeof (unsigned long int));
  if (__hurd_sigthread_variables == NULL)
    __libc_fatal ("hurd: Can't allocate thread variables for signal thread\n");

  /* Reinitialize the MiG support routines so they will use a per-thread
     variable for the cached reply port.  */
  __mig_init ((void *) __hurd_sigthread_stack_base);

  if (err = __thread_resume (_hurd_msgport_thread))
    __libc_fatal ("hurd: Can't resume signal thread\n");
    
#if 0				/* Don't confuse poor gdb.  */
  /* Receive exceptions on the signal port.  */
  __task_set_special_port (__mach_task_self (),
			   TASK_EXCEPTION_PORT, _hurd_msgport);
#endif
}
				/* XXXX */
/* Reauthenticate with the proc server.  */

static void
reauth_proc (mach_port_t new)
{
  mach_port_t ref, ignore;

  ref = __mach_reply_port ();
  if (! HURD_PORT_USE (&_hurd_ports[INIT_PORT_PROC],
		       __proc_reauthenticate (port, ref,
					      MACH_MSG_TYPE_MAKE_SEND) ||
		       __auth_user_authenticate (new, port, ref,
						 MACH_MSG_TYPE_MAKE_SEND,
						 &ignore))
      && ignore != MACH_PORT_NULL)
    __mach_port_deallocate (__mach_task_self (), ignore);
  __mach_port_destroy (__mach_task_self (), ref);

  (void) &reauth_proc;		/* Silence compiler warning.  */
}
text_set_element (_hurd_reauth_hook, reauth_proc);

/* Like `getenv', but safe for the signal thread to run.
   If the environment is trashed, this will just return NULL.  */

const char *
_hurdsig_getenv (const char *variable)
{
  if (_hurdsig_catch_fault (SIGSEGV))
    /* We bombed in getenv.  */
    return NULL;
  else
    {
      const char *value = getenv (variable);
      /* Fault now if VALUE is a bogus string.  */
      (void) strlen (value);
      _hurdsig_end_catch_fault ();
      return value;
    }
}
