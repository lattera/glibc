/* Copyright (C) 1994, 1995, 1996, 1997 Free Software Foundation, Inc.
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

#include <mach/exc_server.h>
#include <hurd/signal.h>

/* Called by the microkernel when a thread gets an exception.  */

kern_return_t
_S_catch_exception_raise (mach_port_t port,
			  thread_t thread,
			  task_t task,
			  int exception,
			  int code,
			  int subcode)
{
  struct hurd_sigstate *ss;
  int signo;
  struct hurd_signal_detail d;

  if (task != __mach_task_self ())
    /* The sender wasn't the kernel.  */
    return EPERM;

  d.exc = exception;
  d.exc_code = code;
  d.exc_subcode = subcode;

  /* Call the machine-dependent function to translate the Mach exception
     codes into a signal number and subcode.  */
  _hurd_exception2signal (&d, &signo);

  /* Find the sigstate structure for the faulting thread.  */
  __mutex_lock (&_hurd_siglock);
  for (ss = _hurd_sigstates; ss != NULL; ss = ss->next)
    if (ss->thread == thread)
      break;
  __mutex_unlock (&_hurd_siglock);
  if (ss == NULL)
    ss = _hurd_thread_sigstate (thread); /* Allocate a fresh one.  */

  if (__spin_lock_locked (&ss->lock))
    {
      /* Loser.  The thread faulted with its sigstate lock held.  Its
	 sigstate data is now suspect.  So we reset the parts of it which
	 could cause trouble for the signal thread.  Anything else
	 clobbered therein will just hose this user thread, but it's
	 faulting already.

	 This is almost certainly a library bug: unless random memory
	 clobberation caused the sigstate lock to gratuitously appear held,
	 no code should do anything that can fault while holding the
	 sigstate lock.  */

      __spin_unlock (&ss->critical_section_lock);
      ss->context = NULL;
      __spin_unlock (&ss->lock);
    }

  /* Post the signal.  */
  _hurd_internal_post_signal (ss, signo, &d,
			      MACH_PORT_NULL, MACH_MSG_TYPE_PORT_SEND,
			      0);

  return KERN_SUCCESS;
}
