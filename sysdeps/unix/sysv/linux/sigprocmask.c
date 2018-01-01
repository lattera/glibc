/* Copyright (C) 1997-2018 Free Software Foundation, Inc.
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
#include <signal.h>
#include <string.h>  /* Needed for string function builtin redirection.  */
#include <unistd.h>

#include <sysdep.h>
#include <sys/syscall.h>

#include <nptl/pthreadP.h>              /* SIGCANCEL, SIGSETXID */


/* Get and/or change the set of blocked signals.  */
int
__sigprocmask (int how, const sigset_t *set, sigset_t *oset)
{
  sigset_t local_newmask;

  /* The only thing we have to make sure here is that SIGCANCEL and
     SIGSETXID are not blocked.  */
  if (set != NULL
      && (__builtin_expect (__sigismember (set, SIGCANCEL), 0)
	  || __builtin_expect (__sigismember (set, SIGSETXID), 0)))
    {
      local_newmask = *set;
      __sigdelset (&local_newmask, SIGCANCEL);
      __sigdelset (&local_newmask, SIGSETXID);
      set = &local_newmask;
    }

  return INLINE_SYSCALL (rt_sigprocmask, 4, how, set, oset, _NSIG / 8);
}
libc_hidden_def (__sigprocmask)
weak_alias (__sigprocmask, sigprocmask)
