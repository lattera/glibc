/* Copyright (C) 1994, 1995 Free Software Foundation, Inc.
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

#include <hurd.h>
#include <hurd/signal.h>
#include <hurd/msg.h>
#include <setjmp.h>

/* Handle signal SIGNO in the calling thread.
   If SS is not NULL it is the sigstate for the calling thread;
   SS->lock is held on entry and released before return.  */

void
_hurd_raise_signal (struct hurd_sigstate *ss,
		    int signo, long int sigcode, int sigerror)
{
  if (ss == NULL)
    {
      ss = _hurd_self_sigstate ();
      __spin_lock (&ss->lock);
    }

  /* Mark SIGNO as pending to be delivered.  */
  __sigaddset (&ss->pending, signo);
  ss->pending_data[signo].code = sigcode;
  ss->pending_data[signo].error = sigerror;

  __spin_unlock (&ss->lock);

  /* Send a message to the signal thread so it
     will wake up and check for pending signals.  */
  __msg_sig_post (_hurd_msgport, 0, __mach_task_self ());
}
