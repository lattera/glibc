/* Linux waitpid syscall implementation -- non-cancellable.
   Copyright (C) 2018 Free Software Foundation, Inc.
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
#include <not-cancel.h>

__pid_t
__waitpid_nocancel (__pid_t pid, int *stat_loc, int options)
{
#ifdef __NR_waitpid
  return INLINE_SYSCALL_CALL (waitpid, pid, stat_loc, options);
#else
  return INLINE_SYSCALL_CALL (wait4, pid, stat_loc, options, NULL);
#endif
}
libc_hidden_def (__waitpid_nocancel)
