/* Poll system call.
   Copyright (C) 1997-2012 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <sys/poll.h>

#include <sysdep-cancel.h>
#include <sys/syscall.h>
#include <bp-checks.h>

#include <kernel-features.h>

int
__poll (fds, nfds, timeout)
     struct pollfd *fds;
     nfds_t nfds;
     int timeout;
{
  if (SINGLE_THREAD_P)
    return INLINE_SYSCALL (poll, 3, CHECK_N (fds, nfds), nfds, timeout);

  int oldtype = LIBC_CANCEL_ASYNC ();

  int result = INLINE_SYSCALL (poll, 3, CHECK_N (fds, nfds), nfds, timeout);

   LIBC_CANCEL_RESET (oldtype);

  return result;
}
libc_hidden_def (__poll)
weak_alias (__poll, poll)
strong_alias (__poll, __libc_poll)
