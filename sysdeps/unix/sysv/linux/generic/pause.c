/* Copyright (C) 2011 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Chris Metcalf <cmetcalf@tilera.com>, 2011.

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

#include <signal.h>
#include <unistd.h>
#include <sysdep-cancel.h>

/* Suspend the process until a signal arrives.
   This always returns -1 and sets errno to EINTR.  */

static int
__syscall_pause (void)
{
  sigset_t set;

  int rc =
    INLINE_SYSCALL (rt_sigprocmask, 4, SIG_BLOCK, NULL, &set, _NSIG / 8);
  if (rc == 0)
    rc = INLINE_SYSCALL (rt_sigsuspend, 2, &set, _NSIG / 8);

  return rc;
}

int
__libc_pause (void)
{
  if (SINGLE_THREAD_P)
    return __syscall_pause ();

  int oldtype = LIBC_CANCEL_ASYNC ();

  int result = __syscall_pause ();

  LIBC_CANCEL_RESET (oldtype);

  return result;
}
weak_alias (__libc_pause, pause)

#ifndef NO_CANCELLATION
# include <not-cancel.h>

int
__pause_nocancel (void)
{
  return __syscall_pause ();
}
#endif
