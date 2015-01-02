/* Determine calling thread's scheduling parameters.  Linux version.
   Copyright (C) 2014-2015 Free Software Foundation, Inc.
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
   not, see <http://www.gnu.org/licenses/>.  */

#include <sysdep.h>

/* This should fill in PD->schedpolicy if PD->flags does not contain
   ATTR_FLAG_POLICY_SET, and set it; and PD->schedparam if PD->flags does
   not contain ATTR_FLAG_SCHED_SET, and set it.  It won't be called at all
   if both bits are already set.  */

static void
collect_default_sched (struct pthread *pd)
{
  INTERNAL_SYSCALL_DECL (scerr);

  if ((pd->flags & ATTR_FLAG_POLICY_SET) == 0)
    {
      pd->schedpolicy = INTERNAL_SYSCALL (sched_getscheduler, scerr, 1, 0);
      pd->flags |= ATTR_FLAG_POLICY_SET;
    }

  if ((pd->flags & ATTR_FLAG_SCHED_SET) == 0)
    {
      INTERNAL_SYSCALL (sched_getparam, scerr, 2, 0, &pd->schedparam);
      pd->flags |= ATTR_FLAG_SCHED_SET;
    }
}
