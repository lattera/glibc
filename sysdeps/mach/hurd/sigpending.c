/* Copyright (C) 1991, 1993, 1994, 1995, 1997 Free Software Foundation, Inc.
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

#include <errno.h>
#include <stddef.h>
#include <hurd.h>
#include <hurd/signal.h>


/* Store in SET all signals that are blocked and pending.  */
/* XXX should be __sigpending ? */
int
sigpending (set)
     sigset_t *set;
{
  struct hurd_sigstate *ss;
  sigset_t pending;

  if (set == NULL)
    {
      errno = EINVAL;
      return -1;
    }

  ss = _hurd_self_sigstate ();
  __spin_lock (&ss->lock);
  pending = ss->pending;
  __spin_unlock (&ss->lock);

  *set = pending;
  return 0;
}
