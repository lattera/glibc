/* Copyright (C) 1994 Free Software Foundation, Inc.
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

/* Initialize PREEMPTER with the information given and stick it in the
   chain of preempters for SIGNO.  */

int
hurd_preempt_signals (struct hurd_signal_preempt *preempter,
		      int signo, int first_code, int last_code,
		      sighandler_t (*handler) (thread_t, int, long int, int))
{
  if (signo <= 0 || signo >= NSIG)
    {
      errno = EINVAL;
      return -1;
    }
  preempter->first = first_code;
  preempter->last = last_code;
  preempter->handler = handler;
  __mutex_lock (&_hurd_signal_preempt_lock);
  preempter->next = _hurd_signal_preempt[signo];
  _hurd_signal_preempt[signo] = preempter;
  __mutex_unlock (&_hurd_signal_preempt_lock);
  return 0;
}

/* Remove PREEMPTER from the chain for SIGNO.  */

int
hurd_unpreempt_signals (struct hurd_signal_preempt *preempter, int signo)
{
  struct hurd_signal_preempt *p, *lastp;
  if (signo <= 0 || signo >= NSIG)
    {
      errno = EINVAL;
      return -1;
    }
  __mutex_lock (&_hurd_signal_preempt_lock);
  for (p = _hurd_signal_preempt[signo], lastp = NULL;
       p != NULL; lastp = p, p = p->next)
    if (p == preempter)
      {
	(lastp == NULL ? _hurd_signal_preempt[signo] : lastp->next) = p->next;
	__mutex_unlock (&_hurd_signal_preempt_lock);
	return 0;
      }
  _hurd_signal_preempt[signo] = preempter;
  __mutex_unlock (&_hurd_signal_preempt_lock);
  errno = ENOENT;
  return -1;
}
