/* Copyright (C) 2011 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Chris Metcalf <cmetcalf@tilera.com>, 2011.

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

#include <stdlib.h>
#include <sys/types.h>
#include <sysdep-cancel.h>
#include <libc-symbols.h>

ssize_t
__libc_send (int sockfd, const void *buffer, size_t len, int flags)
{
  ssize_t result;

  if (SINGLE_THREAD_P)
    result = INLINE_SYSCALL (sendto, 6, sockfd, buffer, len, flags, NULL, 0);
  else
    {
      int oldtype = LIBC_CANCEL_ASYNC ();

      result = INLINE_SYSCALL (sendto, 6, sockfd, buffer, len, flags, NULL, 0);

      LIBC_CANCEL_RESET (oldtype);
    }

  return result;
}
strong_alias (__libc_send, __send)
weak_alias (__libc_send, send)
