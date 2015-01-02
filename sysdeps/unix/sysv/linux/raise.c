/* Copyright (C) 2002-2015 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2002.

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
#include <limits.h>
#include <signal.h>
#include <sysdep.h>
#include <nptl/pthreadP.h>


int
raise (sig)
     int sig;
{
  struct pthread *pd = THREAD_SELF;
  pid_t pid = THREAD_GETMEM (pd, pid);
  pid_t selftid = THREAD_GETMEM (pd, tid);
  if (selftid == 0)
    {
      /* This system call is not supposed to fail.  */
#ifdef INTERNAL_SYSCALL
      INTERNAL_SYSCALL_DECL (err);
      selftid = INTERNAL_SYSCALL (gettid, err, 0);
#else
      selftid = INLINE_SYSCALL (gettid, 0);
#endif
      THREAD_SETMEM (pd, tid, selftid);

      /* We do not set the PID field in the TID here since we might be
	 called from a signal handler while the thread executes fork.  */
      pid = selftid;
    }
  else
    /* raise is an async-safe function.  It could be called while the
       fork/vfork function temporarily invalidated the PID field.  Adjust for
       that.  */
    if (__glibc_unlikely (pid <= 0))
      pid = (pid & INT_MAX) == 0 ? selftid : -pid;

  return INLINE_SYSCALL (tgkill, 3, pid, selftid, sig);
}
libc_hidden_def (raise)
weak_alias (raise, gsignal)
