/* Copyright (C) 2000, 2002 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>

#include <nptl/pthreadP.h>
#include <tls.h>
#include <sysdep.h>
#include <sys/syscall.h>
#include "../kernel-features.h"

extern int __syscall_fcntl64 (int __fd, int __cmd, ...);


int
__libc_fcntl (int fd, int cmd, ...)
{
  va_list ap;

  va_start (ap, cmd);
  void *arg = va_arg (ap, void *);
  va_end (ap);

#ifndef NOT_IN_libc
  if (cmd != F_SETLKW
      || __builtin_expect (THREAD_GETMEM (THREAD_SELF,
					  header.data.multiple_threads) == 0,
			   1))
    return INLINE_SYSCALL (fcntl64, 3, fd, cmd, arg);

  int oldtype = LIBC_CANCEL_ASYNC ();
#endif

  int result = INLINE_SYSCALL (fcntl64, 3, fd, cmd, arg);

#ifndef NOT_IN_libc
  LIBC_CANCEL_RESET (oldtype);
#endif

  return result;
}
libc_hidden_def (__libc_fcntl)

weak_alias (__libc_fcntl, __fcntl)
libc_hidden_weak (__fcntl)
weak_alias (__libc_fcntl, fcntl)
