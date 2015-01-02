/* Copyright (C) 1991-2015 Free Software Foundation, Inc.
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
#include <sysdep-cancel.h>
#include <stdlib.h>
#include <sys/wait.h>

__pid_t
__waitpid (__pid_t pid, int *stat_loc, int options)
{
  if (SINGLE_THREAD_P)
    {
#ifdef __NR_waitpid
      return INLINE_SYSCALL (waitpid, 3, pid, stat_loc, options);
#else
      return INLINE_SYSCALL (wait4, 4, pid, stat_loc, options, NULL);
#endif
    }

  int oldtype = LIBC_CANCEL_ASYNC ();

#ifdef __NR_waitpid
  int result = INLINE_SYSCALL (waitpid, 3, pid, stat_loc, options);
#else
  int result = INLINE_SYSCALL (wait4, 4, pid, stat_loc, options, NULL);
#endif

  LIBC_CANCEL_RESET (oldtype);

  return result;
}
libc_hidden_def (__waitpid)
weak_alias (__waitpid, waitpid)
