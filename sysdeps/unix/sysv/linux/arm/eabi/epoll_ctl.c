/* epoll_ctl wrapper for ARM EABI.
   Copyright (C) 2005 Free Software Foundation, Inc.
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

#include <sysdep.h>
#include <errno.h>
#include <sys/epoll.h>

#include <kernel_epoll.h>

int
epoll_ctl (int __epfd, int __op, int __fd, struct epoll_event *__event)
{
  struct kernel_epoll_event k_event;

  k_event.events = __event->events;
  memcpy (&k_event.data, &__event->data, sizeof (k_event.data));

  return INLINE_SYSCALL (epoll_ctl, 4, __epfd, __op, __fd, &k_event);
}

libc_hidden_def (epoll_ctl)
