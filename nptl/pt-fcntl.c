/* Copyright (C) 2002 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sysdep.h>
#include <unistd.h>
#include "pthreadP.h"


int
__fcntl (int fd, int cmd, ...)
{
  int oldtype = 0;
  va_list ap;

  if (cmd == F_SETLKW)
    oldtype = CANCEL_ASYNC ();

  va_start (ap, cmd);

#ifdef INLINE_SYSCALL
  int result = INLINE_SYSCALL (fcntl, 3, fd, cmd, va_arg (ap, long int));
#else
  int result = __libc_fcntl (fd, cmd, va_arg (ap, long int));
#endif

  va_end (ap);

  if (cmd == F_SETLKW)
    CANCEL_RESET (oldtype);

  return result;
}
strong_alias (__fcntl, fcntl)
