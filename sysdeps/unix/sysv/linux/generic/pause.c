/* Copyright (C) 2011-2016 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include <signal.h>
#include <unistd.h>
#include <sysdep-cancel.h>

/* Suspend the process until a signal arrives.
   This always returns -1 and sets errno to EINTR.  */

int
__libc_pause (void)
{
  sigset_t set;

  int rc =
    SYSCALL_CANCEL (rt_sigprocmask, SIG_BLOCK, NULL, &set, _NSIG / 8);
  if (rc == 0)
    rc = SYSCALL_CANCEL (rt_sigsuspend, &set, _NSIG / 8);

  return rc;
}

weak_alias (__libc_pause, pause)
