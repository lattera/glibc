/* Poll system call, with emulation if it is not available.
   Copyright (C) 1997,1998,1999,2000,2001,2002 Free Software Foundation, Inc.
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
#include <sys/poll.h>

#include <sysdep.h>
#include <sys/syscall.h>
#include <bp-checks.h>
#include <nptl/pthreadP.h>
#include <tls.h>


extern int __syscall_poll (struct pollfd *__unbounded fds,
			   unsigned int nfds, int timeout);


/* The real implementation.  */
int
__poll (fds, nfds, timeout)
     struct pollfd *fds;
     nfds_t nfds;
     int timeout;
{
#ifndef NOT_IN_libc
  if (__builtin_expect (THREAD_GETMEM (THREAD_SELF,
				       header.data.multiple_threads) == 0, 1))
    return INLINE_SYSCALL (poll, 3, CHECK_N (fds, nfds), nfds, timeout);

  int oldtype = LIBC_CANCEL_ASYNC ();
#endif

  int result = INLINE_SYSCALL (poll, 3, CHECK_N (fds, nfds), nfds, timeout);

#ifndef NOT_IN_libc
  LIBC_CANCEL_RESET (oldtype);
#endif

  return result;
}
libc_hidden_def (__poll)
weak_alias (__poll, poll)
strong_alias (__poll, __libc_poll)
