/* Copyright (C) 2002-2012 Free Software Foundation, Inc.
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
#include <signal.h>
#include <sysdep.h>
#include <tls.h>
#include <kernel-features.h>


int
raise (sig)
     int sig;
{
  /* raise is an async-safe function.  It could be called while the
     fork function temporarily invalidated the PID field.  Adjust for
     that.  */
  pid_t pid = THREAD_GETMEM (THREAD_SELF, pid);
  if (__builtin_expect (pid < 0, 0))
    pid = -pid;

  return INLINE_SYSCALL (tgkill, 3, pid, THREAD_GETMEM (THREAD_SELF, tid),
			 sig);
}
