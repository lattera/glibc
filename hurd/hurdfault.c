/* Handle faults in the signal thread.
   Copyright (C) 1994, 1995, 1996, 1997 Free Software Foundation, Inc.
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
#include "hurdfault.h"
#include <errno.h>
#include <string.h>
#include <setjmp.h>
#include <stdio.h>
#include "thread_state.h"
#include "faultexc_server.h"	/* mig-generated header for our exc server.  */
#include <assert.h>

jmp_buf _hurdsig_fault_env;
struct hurd_signal_preemptor _hurdsig_fault_preemptor = {0};

/* XXX temporary to deal with spelling fix */
weak_alias (_hurdsig_fault_preemptor, _hurdsig_fault_preempter)

static mach_port_t forward_sigexc;

kern_return_t
_hurdsig_fault_catch_exception_raise (mach_port_t port,
				      thread_t thread,
				      task_t task,
				      int exception,
				      int code,
				      int subcode)
{
  int signo;
  struct hurd_signal_detail d;

  if (port != forward_sigexc ||
      thread != _hurd_msgport_thread || task != __mach_task_self ())
    return EPERM;		/* Strange bogosity.  */

  d.exc = exception;
  d.exc_code = code;
  d.exc_subcode = subcode;

  /* Call the machine-dependent function to translate the Mach exception
     codes into a signal number and subcode.  */
  _hurd_exception2signal (&d, &signo);

  return HURD_PREEMPT_SIGNAL_P (&_hurdsig_fault_preemptor, signo, d.code)
    ? 0 : EGREGIOUS;
}

static void
faulted (void)
{
  struct
    {
      mach_msg_header_t head;
      char buf[64];
    } request;
  struct
    {
      mach_msg_header_t head;
      mach_msg_type_t type;
      int result;
    } reply;
  extern int _hurdsig_fault_exc_server (mach_msg_header_t *,
					mach_msg_header_t *);

 /* Wait for the exception_raise message forwarded by the proc server.  */

 if (__mach_msg (&request.head, MACH_RCV_MSG, 0,
		  sizeof request, forward_sigexc,
		  MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL)
      != MACH_MSG_SUCCESS)
    __libc_fatal ("msg receive failed on signal thread exc\n");

  /* Run the exc demuxer which should call the server function above.
     That function returns 0 if the exception was expected.  */
  _hurdsig_fault_exc_server (&request.head, &reply.head);
  if (reply.head.msgh_remote_port != MACH_PORT_NULL)
    __mach_msg (&reply.head, MACH_SEND_MSG, reply.head.msgh_size,
		0, MACH_PORT_NULL, MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
  if (reply.result == MIG_BAD_ID)
    __mach_msg_destroy (&request.head);

  if (reply.result)
    __libc_fatal ("BUG: unexpected fault in signal thread\n");

  _hurdsig_fault_preemptor.signals = 0;
  longjmp (_hurdsig_fault_env, 1);
}

static char faultstack[1024];

/* Send exceptions for the signal thread to the proc server.
   It will forward the message on to our message port,
   and then restore the thread's state to code which
   does `longjmp (_hurd_sigthread_fault_env, 1)'.  */

void
_hurdsig_fault_init (void)
{
  error_t err;
  struct machine_thread_state state;
  mach_port_t sigexc;

  /* Allocate a port to receive signal thread exceptions.
     We will move this receive right to the proc server.  */
  err = __mach_port_allocate (__mach_task_self (),
			      MACH_PORT_RIGHT_RECEIVE, &sigexc);
  assert_perror (err);
  err = __mach_port_allocate (__mach_task_self (),
			      MACH_PORT_RIGHT_RECEIVE, &forward_sigexc);
  assert_perror (err);

  /* Allocate a port to receive the exception msgs forwarded
     from the proc server.  */
  err = __mach_port_insert_right (__mach_task_self (), sigexc,
				  sigexc, MACH_MSG_TYPE_MAKE_SEND);
  assert_perror (err);

  /* Set the queue limit for this port to just one.  The proc server will
     notice if we ever get a second exception while one remains queued and
     unreceived, and decide we are hopelessly buggy.  */
  err = __mach_port_set_qlimit (__mach_task_self (), forward_sigexc, 1);
  assert_perror (err);

  /* This state will be restored when we fault.
     It runs the function above.  */
  memset (&state, 0, sizeof state);
  MACHINE_THREAD_STATE_SET_PC (&state, faulted);
  MACHINE_THREAD_STATE_SET_SP (&state, faultstack, sizeof faultstack);

  err = __USEPORT
    (PROC,
     __proc_handle_exceptions (port,
			       sigexc,
			       forward_sigexc, MACH_MSG_TYPE_MAKE_SEND,
			       MACHINE_THREAD_STATE_FLAVOR,
			       (natural_t *) &state,
			       MACHINE_THREAD_STATE_COUNT));
  assert_perror (err);

  /* Direct signal thread exceptions to the proc server.  */
  err = __thread_set_special_port (_hurd_msgport_thread,
				   THREAD_EXCEPTION_PORT, sigexc);
  __mach_port_deallocate (__mach_task_self (), sigexc);
  assert_perror (err);
}
