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

#include <ansidecl.h>
#include <errno.h>
#include <signal.h>
#include <hurd.h>
#include <hurd/signal.h>
#include <hurd/msg.h>


/* If SET is not NULL, modify the current set of blocked signals
   according to HOW, which may be SIG_BLOCK, SIG_UNBLOCK or SIG_SETMASK.
   If OSET is not NULL, store the old set of blocked signals in *OSET.  */
int
DEFUN(__sigprocmask, (how, set, oset),
      int how AND CONST sigset_t *set AND sigset_t *oset)
{
  struct hurd_sigstate *ss;
  sigset_t old, new;
  sigset_t pending;

  if (set != NULL)
    new = *set;

  ss = _hurd_self_sigstate ();

  __spin_lock (&ss->lock);

  old = ss->blocked;

  if (set != NULL)
    {
      switch (how)
	{
	case SIG_BLOCK:
	  ss->blocked |= new;
	  break;

	case SIG_UNBLOCK:
	  ss->blocked &= ~new;
	  break;

	case SIG_SETMASK:
	  ss->blocked = new;
	  break;

	default:
	  __spin_unlock (&ss->lock);
	  errno = EINVAL;
	  return -1;
	}

      ss->blocked &= ~_SIG_CANT_MASK;
    }

  pending = ss->pending & ~ss->blocked;

  __spin_unlock (&ss->lock);

  if (oset != NULL)
    *oset = old;

  if (pending)
    /* Send a message to the signal thread so it
       will wake up and check for pending signals.  */
    __msg_sig_post (_hurd_msgport, 0, __mach_task_self ());

  return 0;
}

weak_alias (__sigprocmask, sigprocmask)
