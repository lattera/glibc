/* Linux select implementation.
   Copyright (C) 2017-2018 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <errno.h>
#include <sysdep-cancel.h>

/* Check the first NFDS descriptors each in READFDS (if not NULL) for read
   readiness, in WRITEFDS (if not NULL) for write readiness, and in EXCEPTFDS
   (if not NULL) for exceptional conditions.  If TIMEOUT is not NULL, time out
   after waiting the interval specified therein.  Returns the number of ready
   descriptors, or -1 for errors.  */

#ifdef __NR__newselect
# undef __NR_select
# define __NR_select __NR__newselect
#endif

int
__select (int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
	  struct timeval *timeout)
{
#ifdef __NR_select
  return SYSCALL_CANCEL (select, nfds, readfds, writefds, exceptfds,
			 timeout);
#else
  int result;
  struct timespec ts, *tsp = NULL;

  if (timeout)
    {
      TIMEVAL_TO_TIMESPEC (timeout, &ts);
      tsp = &ts;
    }

  result = SYSCALL_CANCEL (pselect6, nfds, readfds, writefds, exceptfds, tsp,
			   NULL);

  if (timeout)
    {
      /* Linux by default will update the timeout after a pselect6 syscall
         (though the pselect() glibc call suppresses this behavior).
         Since select() on Linux has the same behavior as the pselect6
         syscall, we update the timeout here.  */
      TIMESPEC_TO_TIMEVAL (timeout, &ts);
    }

  return result;
#endif
}
libc_hidden_def (__select)

weak_alias (__select, select)
weak_alias (__select, __libc_select)
