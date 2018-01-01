/* Copyright (C) 2006-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2006.

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
#include <signal.h>
#include <time.h>
#include <sys/poll.h>
#include <kernel-features.h>
#include <sysdep-cancel.h>


#ifdef __NR_pselect6
# ifndef __ASSUME_PSELECT
static int __generic_pselect (int nfds, fd_set *readfds, fd_set *writefds,
			      fd_set *exceptfds,
			      const struct timespec *timeout,
			      const sigset_t *sigmask);
# endif


int
__pselect (int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
	   const struct timespec *timeout, const sigset_t *sigmask)
{
  /* The Linux kernel can in some situations update the timeout value.
     We do not want that so use a local variable.  */
  struct timespec tval;
  if (timeout != NULL)
    {
      tval = *timeout;
      timeout = &tval;
    }

  /* Note: the system call expects 7 values but on most architectures
     we can only pass in 6 directly.  If there is an architecture with
     support for more parameters a new version of this file needs to
     be created.  */
  struct
  {
    __syscall_ulong_t ss;
    __syscall_ulong_t ss_len;
  } data;

  data.ss = (__syscall_ulong_t) (uintptr_t) sigmask;
  data.ss_len = _NSIG / 8;

  int result;

#ifndef CALL_PSELECT6
# define CALL_PSELECT6(nfds, readfds, writefds, exceptfds, timeout, data) \
  SYSCALL_CANCEL (pselect6, nfds, readfds, writefds, exceptfds,	timeout, data)
#endif

  result = CALL_PSELECT6 (nfds, readfds, writefds, exceptfds, timeout,
			  &data);

# ifndef __ASSUME_PSELECT
  if (result == -1 && errno == ENOSYS)
    result = __generic_pselect (nfds, readfds, writefds, exceptfds, timeout,
				sigmask);
# endif

  return result;
}
weak_alias (__pselect, pselect)

# ifndef __ASSUME_PSELECT
#  define __pselect static __generic_pselect
# endif
#endif

#ifndef __ASSUME_PSELECT
# include <misc/pselect.c>
#endif
