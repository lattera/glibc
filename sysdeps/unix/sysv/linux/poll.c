/* Poll system call, with emulation if it is not available.
   Copyright (C) 1997, 1998, 1999, 2000, 2001 Free Software Foundation, Inc.
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
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <errno.h>
#include <sys/poll.h>

#include <sysdep.h>
#include <sys/syscall.h>
#include <bp-checks.h>

#include "kernel-features.h"

#if defined __NR_poll || __ASSUME_POLL_SYSCALL > 0

extern int __syscall_poll (struct pollfd *__unbounded fds,
			   unsigned int nfds, int timeout);

# if __ASSUME_POLL_SYSCALL == 0
static int __emulate_poll (struct pollfd *fds, nfds_t nfds,
			   int timeout) internal_function;
# endif

/* The real implementation.  */
int
__poll (fds, nfds, timeout)
     struct pollfd *fds;
     nfds_t nfds;
     int timeout;
{
# if __ASSUME_POLL_SYSCALL == 0
  static int must_emulate;

  if (!must_emulate)
    {
      int errno_saved = errno;
      int retval = INLINE_SYSCALL (poll, 3, CHECK_N (fds, nfds), nfds, timeout);

      if (retval >= 0 || errno != ENOSYS)
	return retval;

      __set_errno (errno_saved);
      must_emulate = 1;
    }

  return __emulate_poll (fds, nfds, timeout);
# else
  return INLINE_SYSCALL (poll, 3, CHECK_N (fds, nfds), nfds, timeout);
# endif
}
weak_alias (__poll, poll)

/* Get the emulation code.  */
# define __poll(fds, nfds, timeout) \
  static internal_function __emulate_poll (fds, nfds, timeout)
#endif

#if __ASSUME_POLL_SYSCALL == 0
# include <sysdeps/unix/bsd/poll.c>
#endif
