/* Implementing POSIX.1 signals under the Hurd.
Copyright (C) 1993, 1994, 1995 Free Software Foundation, Inc.
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

#ifndef	_HURD_SIGNAL_H

#define	_HURD_SIGNAL_H	1
#include <features.h>
/* Make sure <signal.h> is going to define NSIG.  */
#ifndef __USE_GNU
#error "Must have `_GNU_SOURCE' feature test macro to use this file"
#endif

#define __need_NULL
#include <stddef.h>

#include <mach/mach_types.h>
#include <mach/port.h>
#include <mach/message.h>
#include <hurd/hurd_types.h>
#include <signal.h>
#include <errno.h>
#include <hurd/msg.h>

#include <cthreads.h>		/* For `struct mutex'.  */
#include <spin-lock.h>
#include <hurd/threadvar.h>	/* We cache sigstate in a threadvar.  */


/* Per-thread signal state.  */

struct hurd_sigstate
  {
    spin_lock_t lock;		/* Locks most of the rest of the structure.  */

    int critical_section;	/* Nonzero if in critical section.  */

    thread_t thread;
    struct hurd_sigstate *next; /* Linked-list of thread sigstates.  */

    sigset_t blocked;		/* What signals are blocked.  */
    sigset_t pending;		/* Pending signals, possibly blocked.  */
    struct sigaction actions[NSIG];
    struct sigaltstack sigaltstack;
    struct
      {
	/* For each signal that may be pending, the
	   sigcode and error code to deliver it with.  */
	long int code;
	error_t error;
      } pending_data[NSIG];

    /* If `suspended' is set when this thread gets a signal,
       the signal thread sends an empty message to it.  */
    mach_port_t suspended;

    /* The following members are not locked.  They are used only by this
       thread, or by the signal thread with this thread suspended.  */

    volatile mach_port_t intr_port; /* Port interruptible RPC was sent on.  */

    /* If this is not null, the thread is in sigreturn awaiting delivery of
       pending signals.  This context (the machine-dependent portions only)
       will be passed to sigreturn after running the handler for a pending
       signal, instead of examining the thread state.  */
    struct sigcontext *context;
  };

/* Linked list of states of all threads whose state has been asked for.  */

extern struct hurd_sigstate *_hurd_sigstates;

extern struct mutex _hurd_siglock; /* Locks _hurd_sigstates.  */

/* Get the sigstate of a given thread, taking its lock.  */

extern struct hurd_sigstate *_hurd_thread_sigstate (thread_t);

/* Get the sigstate of the current thread.
   This uses a per-thread variable to optimize the lookup.  */
_EXTERN_INLINE struct hurd_sigstate *
_hurd_self_sigstate (void)
{
  struct hurd_sigstate **location =
    (void *) __hurd_threadvar_location (_HURD_THREADVAR_SIGSTATE);
  if (*location == NULL)
    *location = _hurd_thread_sigstate (__mach_thread_self ());
  return *location;
}

/* Thread listening on our message port; also called the "signal thread".  */

extern thread_t _hurd_msgport_thread;

/* Our message port.  We hold the receive right and _hurd_msgport_thread
   listens for messages on it.  We also hold a send right, for convenience.  */

extern mach_port_t _hurd_msgport;


/* Thread to receive process-global signals.  */

extern thread_t _hurd_sigthread;


/* Resource limit on core file size.  Enforced by hurdsig.c.  */
extern int _hurd_core_limit;

/* Critical sections.

   A critical section is a section of code which cannot safely be interrupted
   to run a signal handler; for example, code that holds any lock cannot be
   interrupted lest the signal handler try to take the same lock and
   deadlock result.  */

_EXTERN_INLINE void *
_hurd_critical_section_lock (void)
{
  struct hurd_sigstate **location =
    (void *) __hurd_threadvar_location (_HURD_THREADVAR_SIGSTATE);
  struct hurd_sigstate *ss = *location;
  if (ss == NULL)
    /* The thread variable is unset; this must be the first time we've
       asked for it.  In this case, the critical section flag cannot
       possible already be set.  Look up our sigstate structure the slow
       way; this locks the sigstate lock.  */
    ss = *location = _hurd_thread_sigstate (__mach_thread_self ());
  else
    __spin_lock (&ss->lock);

  if (ss->critical_section)
    {
      /* We are already in a critical section, so do nothing.  */
      __spin_unlock (&ss->lock);
      return NULL;
    }

  /* Set the critical section flag so no signal handler will run.  */
  ss->critical_section = 1;
  __spin_unlock (&ss->lock);

  /* Return our sigstate pointer; this will be passed to
     _hurd_critical_section_unlock to clear the critical section flag. */
  return ss;
}

_EXTERN_INLINE void
_hurd_critical_section_unlock (void *our_lock)
{
  if (our_lock == NULL)
    /* The critical section lock was held when we began.  Do nothing.  */
    return;
  else
    {
      /* It was us who acquired the critical section lock.  Clear the
	 critical section flag.  */
      struct hurd_sigstate *ss = our_lock;
      sigset_t pending;
      __spin_lock (&ss->lock);
      ss->critical_section = 0;
      pending = ss->pending & ~ss->blocked;
      __spin_unlock (&ss->lock);
      if (pending)
	/* There are unblocked signals pending, which weren't
	   delivered because we were in the critical section.
	   Tell the signal thread to deliver them now.  */
	__msg_sig_post (_hurd_msgport, 0, __mach_task_self ());
    }
}

/* Convenient macros for simple uses of critical sections.
   These two must be used as a pair at the same C scoping level.  */

#define HURD_CRITICAL_BEGIN \
  { void *__hurd_critical__ = _hurd_critical_section_lock ()
#define HURD_CRITICAL_END \
      _hurd_critical_section_unlock (__hurd_critical__); } while (0)

/* Initialize the signal code, and start the signal thread.  */

extern void _hurdsig_init (void);

/* Initialize proc server-assisted fault recovery for the signal thread.  */

extern void _hurdsig_fault_init (void);

/* Raise a signal as described by SIGNO, SIGCODE and SIGERROR, on the
   thread whose sigstate SS points to.  If SS is a null pointer, this
   instead affects the calling thread.  */

extern void _hurd_raise_signal (struct hurd_sigstate *ss,
				int signo, long int sigcode, int sigerror);

/* Translate a Mach exception into a signal (machine-dependent).  */

extern void _hurd_exception2signal (int exception, int code, int subcode,
				    int *signo, long int *sigcode, int *error);


/* Make the thread described by SS take the signal described by SIGNO and
   SIGCODE.  If the process is traced, this will in fact stop with a SIGNO
   as the stop signal unless UNTRACED is nonzero.  When the signal can be
   considered delivered, sends a sig_post reply message on REPLY_PORT
   indicating success.  SS is not locked.  */

extern void _hurd_internal_post_signal (struct hurd_sigstate *ss,
					int signo, long int sigcode, int error,
					mach_port_t reply_port,
					mach_msg_type_name_t reply_port_type,
					int untraced);

/* Set up STATE and SS to handle signal SIGNO by running HANDLER.  If
   RPC_WAIT is nonzero, the thread needs to wait for a pending RPC to
   finish before running the signal handler.  The handler is passed SIGNO,
   SIGCODE, and the returned `struct sigcontext' (which resides on the
   stack the handler will use, and which describes the state of the thread
   encoded in STATE before running the handler).  */

struct machine_thread_all_state;
extern struct sigcontext *
_hurd_setup_sighandler (struct hurd_sigstate *ss, __sighandler_t handler,
			int signo, long int sigcode,
			int rpc_wait, struct machine_thread_all_state *state);

/* Function run by the signal thread to receive from the signal port.  */

extern void _hurd_msgport_receive (void);

/* STATE describes a thread that had intr_port set (meaning it was inside
   HURD_EINTR_RPC), after it has been thread_abort'd.  It it looks to have
   just completed a mach_msg_trap system call that returned
   MACH_RCV_INTERRUPTED, return nonzero and set *PORT to the receive right
   being waited on.  */

extern int _hurdsig_rcv_interrupted_p (struct machine_thread_all_state *state,
				       mach_port_t *port);

/* Set up STATE with a thread state that, when resumed, is
   like `longjmp (_hurd_sigthread_fault_env, 1)'.  */

extern void _hurd_initialize_fault_recovery_state (void *state);


/* Function run for SIGINFO when its action is SIG_DFL and the current
   process is the session leader.  */

extern void _hurd_siginfo_handler (int);


/* Perform interruptible RPC CALL on PORT.
   The call should use 
   The args in CALL should be constant or local variable refs.
   They may be evaluated many times, and must not change.
   PORT must not be deallocated before this RPC is finished.  */
#define	HURD_EINTR_RPC(port, call)					      \
  ({									      \
    __label__ __do_call;	/* Give this label block scope.  */	      \
    error_t __err;							      \
    struct hurd_sigstate *__ss = _hurd_self_sigstate ();		      \
    __do_call:								      \
    /* Tell the signal thread that we are doing an interruptible RPC on	      \
       this port.  If we get a signal and should return EINTR, the signal     \
       thread will set this variable to MACH_PORT_NULL.  The RPC might	      \
       return EINTR when some other thread gets a signal, in which case we    \
       want to restart our call.  */					      \
    __ss->intr_port = (port);						      \
    /* A signal may arrive here, after intr_port is set, but before the	      \
       mach_msg system call.  The signal handler might do an interruptible    \
       RPC, and clobber intr_port; then it would not be set properly when     \
       we actually did send the RPC, and a later signal wouldn't interrupt    \
       that RPC.  So, _hurd_setup_sighandler saves intr_port in the	      \
       sigcontext, and sigreturn restores it.  */			      \
    switch (__err = (call))						      \
      {									      \
      case EINTR:		/* RPC went out and was interrupted.  */      \
      case MACH_SEND_INTERRUPTED: /* RPC didn't get out.  */		      \
	if (__ss->intr_port != MACH_PORT_NULL)				      \
	  /* If this signal was for us and it should interrupt calls, the     \
             signal thread will have cleared SS->intr_port.  Since it's not   \
             cleared, the signal was for another thread, or SA_RESTART is     \
             set.  Restart the interrupted call.  */			      \
	  goto __do_call;						      \
	/* FALLTHROUGH */						      \
      case MACH_RCV_PORT_DIED:						      \
	/* Server didn't respond to interrupt_operation,		      \
	   so the signal thread destroyed the reply port.  */		      \
	__err = EINTR;							      \
	break;								      \
      default:			/* Quiet -Wswitch-enum.  */		      \
      }									      \
    __ss->intr_port = MACH_PORT_NULL;					      \
    __err;								      \
  })									      \


/* Mask of signals that cannot be caught, blocked, or ignored.  */
#define	_SIG_CANT_MASK	(__sigmask (SIGSTOP) | __sigmask (SIGKILL))

/* Do an RPC to a process's message port.

   Each argument is an expression which returns an error code; each
   expression may be evaluated several times.  FETCH_MSGPORT_EXPR should
   fetch the appropriate message port and store it in the local variable
   `msgport'; it will be deallocated after use.  FETCH_REFPORT_EXPR should
   fetch the appropriate message port and store it in the local variable
   `refport' (if no reference port is needed in the call, then
   FETCH_REFPORT_EXPR should be simply KERN_SUCCESS or 0); if
   DEALLOC_REFPORT evaluates to nonzero it will be deallocated after use,
   otherwise the FETCH_REFPORT_EXPR must take care of user references to
   `refport'.  RPC_EXPR should perform the desired RPC operation using
   `msgport' and `refport'.

   The reason for the complexity is that a process's message port and
   reference port may change between fetching those ports and completing an
   RPC using them (usually they change only when a process execs).  The RPC
   will fail with MACH_SEND_INVALID_DEST if the msgport dies before we can
   send the RPC request; or with MIG_SERVER_DIED if the msgport was
   destroyed after we sent the RPC request but before it was serviced.  In
   either of these cases, we retry the entire operation, discarding the old
   message and reference ports and fetch them anew.  */

#define HURD_MSGPORT_RPC(fetch_msgport_expr,				      \
			 fetch_refport_expr, dealloc_refport,		      \
			 rpc_expr) 					      \
({									      \
    error_t __err;							      \
    mach_port_t msgport, refport = MACH_PORT_NULL;			      \
    do									      \
      {									      \
	/* Get the message port.  */					      \
	if (__err = (fetch_msgport_expr))				      \
	  break;							      \
	/* Get the reference port.  */					      \
	if (__err = (fetch_refport_expr))				      \
	  {								      \
	    /* Couldn't get it; deallocate MSGPORT and fail.  */	      \
	    __mach_port_deallocate (__mach_task_self (), msgport);	      \
	    break;							      \
	  }								      \
	__err = (rpc_expr);						      \
	__mach_port_deallocate (__mach_task_self (), msgport);		      \
	if ((dealloc_refport) && refport != MACH_PORT_NULL)		      \
	  __mach_port_deallocate (__mach_task_self (), refport);    	      \
      } while (__err == MACH_SEND_INVALID_DEST ||			      \
	       __err == MIG_SERVER_DIED);				      \
    __err;								      \
})

/* Some other parts of the library need to preempt signals, to detect
   errors that should not result in a POSIX signal.  For example, when
   some mapped region of memory is used, an extraneous SIGSEGV might be
   generated when the mapping server returns an error for a page fault.  */

struct hurd_signal_preempt
  {
    /* Function to examine a thread receiving a given signal.  The handler
       is called even for blocked signals.  This function is run in the
       signal thread, with THREAD's sigstate locked; it should be as simple
       and robust as possible.  THREAD is the thread which is about to
       receive the signal.  SIGNO and SIGCODE would be passed to the normal
       handler.

       If the return value is SIG_DFL, normal signal processing continues.
       If it is SIG_IGN, the signal is ignored.
       Any other value is used in place of the normal handler.  */
    sighandler_t (*handler) (thread_t thread,
			     int signo, long int sigcode, int sigerror);
    long int first, last;	/* Range of sigcodes this handler wants.  */
    struct hurd_signal_preempt *next; /* Next handler on the chain. */
  };

extern struct hurd_signal_preempt *_hurd_signal_preempt[NSIG];
extern struct mutex _hurd_signal_preempt_lock;


#endif	/* hurd/signal.h */
