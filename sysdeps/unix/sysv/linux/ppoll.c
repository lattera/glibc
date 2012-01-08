/* Copyright (C) 2006, 2007, 2012 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <errno.h>
#include <signal.h>
#include <time.h>
#include <sys/poll.h>
#include <kernel-features.h>
#include <sysdep-cancel.h>


#ifdef __NR_ppoll
# ifndef __ASSUME_PPOLL
static int __generic_ppoll (struct pollfd *fds, nfds_t nfds,
			    const struct timespec *timeout,
			    const sigset_t *sigmask);
# endif


int
ppoll (struct pollfd *fds, nfds_t nfds, const struct timespec *timeout,
       const sigset_t *sigmask)
{
  /* The Linux kernel can in some situations update the timeout value.
     We do not want that so use a local variable.  */
  struct timespec tval;
  if (timeout != NULL)
    {
      tval = *timeout;
      timeout = &tval;
    }

  int result;

  if (SINGLE_THREAD_P)
    result = INLINE_SYSCALL (ppoll, 5, fds, nfds, timeout, sigmask, _NSIG / 8);
  else
    {
      int oldtype = LIBC_CANCEL_ASYNC ();

      result = INLINE_SYSCALL (ppoll, 5, fds, nfds, timeout, sigmask,
			       _NSIG / 8);

      LIBC_CANCEL_RESET (oldtype);
    }

# ifndef __ASSUME_PPOLL
  if (result == -1 && errno == ENOSYS)
    result = __generic_ppoll (fds, nfds, timeout, sigmask);
# endif

  return result;
}
libc_hidden_def (ppoll)

# ifndef __ASSUME_PPOLL
#  define ppoll static __generic_ppoll
# endif
#endif

#ifndef __ASSUME_PPOLL
# include <io/ppoll.c>
#endif
