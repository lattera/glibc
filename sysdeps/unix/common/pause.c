/* Copyright (C) 1991, 1996, 2002 Free Software Foundation, Inc.
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

#include <signal.h>
#include <unistd.h>
#include <sysdep-cancel.h>

/* Suspend the process until a signal arrives.
   This always returns -1 and sets errno to EINTR.  */

int
__libc_pause (void)
{
  if (SINGLE_THREAD_P)
    return __sigpause (__sigblock (0), 0);

  int oldtype = LIBC_CANCEL_ASYNC ();
  int result = __sigpause (__sigblock (0), 0);
  LIBC_CANCEL_RESET (oldtype);
  return result;
}
weak_alias (__libc_pause, pause)
