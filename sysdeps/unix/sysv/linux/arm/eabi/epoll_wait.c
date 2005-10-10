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
#include <stdlib.h>

#include <kernel_epoll.h>

int
epoll_wait (int __epfd, struct epoll_event *__events,
	    int __maxevents, int __timeout);
{
  struct kernel_epoll_event *k_events;
  int result;

  k_events = malloc (sizeof (struct kernel_epoll_event) * __maxevents);
  if (k_events == NULL)
    {
      __set_errno (ENOMEM);
      return -1;
    }

  result = INLINE_SYSCALL (epoll_wait, 4, __epfd, __events, k_events,
			   __timeout);

  for (i = 0; i < result; i++)
    {
      __events[i].events = k_events[i].events;
      memcpy (&__events[i].data, &k_events[i].data, sizeof (k_events[i].data));
    }

  free (k_events);
  return result;
}

libc_hidden_def (epoll_wait)
