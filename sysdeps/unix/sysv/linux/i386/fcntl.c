/* Copyright (C) 2000 Free Software Foundation, Inc.
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
#include <fcntl.h>
#include <stdarg.h>

#include <sysdep.h>
#include <sys/syscall.h>
#include "../kernel-features.h"


int
__libc_fcntl (int fd, int cmd, ...)
{
  va_list ap;
#if __NR_fcntl64
  int result;
#endif
  void *arg;

  va_start (ap, cmd);
  arg = va_arg (ap, void *);
  va_end (ap);

#if __NR_fcntl64
  result = INLINE_SYSCALL (fcntl64, 3, fd, cmd, arg);

# if __ASSUME_FCNTL64 == 0
  if (result != -1 || errno != ENOSYS)
# endif
    return result;
#endif

#if __ASSUME_FCNTL64 == 0
  if (cmd == F_GETLK64 || cmd == F_SETLK64 || cmd == F_SETLKW64)
    {
      __set_errno (EINVAL);
      return -1;
    }

  return INLINE_SYSCALL (fcntl, 3, fd, cmd, arg);
#endif
}

weak_alias (__libc_fcntl, __fcntl)
weak_alias (__libc_fcntl, fcntl)
