/* Copyright (C) 2008 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2008.

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
#include <signal.h>
#include <sys/socket.h>

#include <sysdep-cancel.h>
#include <sys/syscall.h>

#ifdef __NR_paccept
int
paccept (int fd, __SOCKADDR_ARG addr, socklen_t *addr_len,
	 const __sigset_t *ss, int flags)
{
  if (SINGLE_THREAD_P)
    return INLINE_SYSCALL (paccept, 6, fd, addr.__sockaddr__, addr_len, ss,
			   _NSIG / 8, flags);

  int oldtype = LIBC_CANCEL_ASYNC ();

  int result = INLINE_SYSCALL (paccept, 6, fd, addr.__sockaddr__, addr_len, ss,
			       _NSIG / 8, flags);

  LIBC_CANCEL_RESET (oldtype);

  return result;
}
#else
int
paccept (int fd, __SOCKADDR_ARG addr, socklen_t *addr_len,
	 const __sigset_t *ss, int flags)
{
  __set_errno (ENOSYS);
  return -1;
stub_warning (epoll_pwait)
}
#endif
