/* Copyright (C) 1994, 1995, 1996 Free Software Foundation, Inc.
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

#include <hurd/sigpreempt.h>
#include <hurd/signal.h>
#include <assert.h>

void
hurd_preempt_signals (struct hurd_signal_preempter *preempter)
{
  __mutex_lock (&_hurd_siglock);
  preempter->next = _hurdsig_preempters;
  _hurdsig_preempters = preempter;
  _hurdsig_preempted_set |= preempter->signals;
  __mutex_unlock (&_hurd_siglock);
}

void
hurd_unpreempt_signals (struct hurd_signal_preempter *preempter)
{
  struct hurd_signal_preempter **p;
  sigset_t preempted = 0;

  __mutex_lock (&_hurd_siglock);

  p = &_hurdsig_preempters;
  while (*p)
    if (*p == preempter)
      {
	/* Found it; take it off the chain.  */
	*p = (*p)->next;
	if ((preempter->signals & preempted) != preempter->signals)
	  {
	    /* This might have been the only preempter for some
	       of those signals, so we must collect the full mask
	       from the others.  */
	    struct hurd_signal_preempter *pp;
	    for (pp = *p; pp; pp = pp->next)
	      preempted |= pp->signals;
	    _hurdsig_preempted_set = preempted;
	  }
	__mutex_unlock (&_hurd_siglock);
	return;
      }
    else
      {
	preempted |= (*p)->signals;
	p = &(*p)->next;
      }

  __mutex_unlock (&_hurd_siglock); /* Avoid deadlock during death rattle.  */
  assert (! "removing absent preempter");
}
