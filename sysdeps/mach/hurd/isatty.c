/* Copyright (C) 1994, 1995, 1997 Free Software Foundation, Inc.
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

#include <errno.h>
#include <unistd.h>
#include <hurd/fd.h>
#include <hurd/term.h>

/* Return 1 if FD is a terminal, 0 if not.  */
int
__isatty (fd)
     int fd;
{
  error_t err;
  mach_port_t id;

  err = HURD_DPORT_USE (fd, __term_getctty (port, &id));
  if (! err)
    __mach_port_deallocate (__mach_task_self (), id);

  return !err;
}

weak_alias (__isatty, isatty)
