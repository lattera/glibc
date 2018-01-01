/* Copyright (C) 2000-2018 Free Software Foundation, Inc.
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

#include <fcntl.h>
#include <stdarg.h>
#include <errno.h>
#include <sysdep-cancel.h>
#include <not-cancel.h>

#ifndef __NR_fcntl64
# define __NR_fcntl64 __NR_fcntl
#endif

#ifndef FCNTL_ADJUST_CMD
# define FCNTL_ADJUST_CMD(__cmd) __cmd
#endif

static int
fcntl_common (int fd, int cmd, void *arg)
{
  if (cmd == F_GETOWN)
    {
      INTERNAL_SYSCALL_DECL (err);
      struct f_owner_ex fex;
      int res = INTERNAL_SYSCALL_CALL (fcntl64, err, fd, F_GETOWN_EX, &fex);
      if (!INTERNAL_SYSCALL_ERROR_P (res, err))
	return fex.type == F_OWNER_GID ? -fex.pid : fex.pid;

      return INLINE_SYSCALL_ERROR_RETURN_VALUE (INTERNAL_SYSCALL_ERRNO (res,
								    err));
    }

  return INLINE_SYSCALL_CALL (fcntl64, fd, cmd, (void *) arg);
}

int
__libc_fcntl (int fd, int cmd, ...)
{
  va_list ap;
  void *arg;

  va_start (ap, cmd);
  arg = va_arg (ap, void *);
  va_end (ap);

  cmd = FCNTL_ADJUST_CMD (cmd);

  if (cmd == F_SETLKW || cmd == F_SETLKW64)
    return SYSCALL_CANCEL (fcntl64, fd, cmd, (void *) arg);

  return fcntl_common (fd, cmd, arg);
}
libc_hidden_def (__libc_fcntl)

#if !IS_IN (rtld)
int
__fcntl_nocancel (int fd, int cmd, ...)
{
  va_list ap;
  void *arg;

  va_start (ap, cmd);
  arg = va_arg (ap, void *);
  va_end (ap);

  return fcntl_common (fd, cmd, arg);
}
#else
strong_alias (__libc_fcntl, __fcntl_nocancel)
#endif
libc_hidden_def (__fcntl_nocancel)

weak_alias (__libc_fcntl, __fcntl)
libc_hidden_weak (__fcntl)
weak_alias (__libc_fcntl, fcntl)
