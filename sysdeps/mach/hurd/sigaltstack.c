/* Copyright (C) 1992, 93, 94, 95, 97, 98 Free Software Foundation, Inc.
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
#include <hurd.h>
#include <hurd/signal.h>

/* Run signals handlers on the stack specified by SS (if not NULL).
   If OSS is not NULL, it is filled in with the old signal stack status.  */
int
__sigaltstack (argss, oss)
     const struct sigaltstack *argss;
     struct sigaltstack *oss;
{
  struct hurd_sigstate *s;
  struct sigaltstack ss, old;

  /* Fault before taking any locks.  */
  if (argss != NULL)
    ss = *argss;
  if (oss != NULL)
    *(volatile struct sigaltstack *) oss = *oss;

  s = _hurd_self_sigstate ();
  __spin_lock (&s->lock);

  if (argss != NULL &&
      (ss.ss_flags & SS_DISABLE) && (s->sigaltstack.ss_flags & SS_ONSTACK))
    {
      /* Can't disable a stack that is in use.  */
      __spin_unlock (&s->lock);
      errno = EINVAL;
      return -1;
    }

  old = s->sigaltstack;

  if (argss != NULL)
    s->sigaltstack = ss;

  __spin_unlock (&s->lock);

  if (oss != NULL)
    *oss = old;

  return 0;
}
weak_alias (__sigaltstack, sigaltstack)
