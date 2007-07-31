/* Copyright (C) 1991-1995, 97, 99, 2000 Free Software Foundation, Inc.
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

#include <sys/time.h>
#include <errno.h>
#include <stddef.h>
#include <hurd.h>

/* Change the access time of FILE to TVP[0] and
   the modification time of FILE to TVP[1].  */
int
__utimes (file, tvp)
     const char *file;
     const struct timeval tvp[2];
{
  struct timeval timevals[2];
  error_t err;
  file_t port;

  if (tvp == NULL)
    {
      /* Setting the number of microseconds to `-1' tells the
         underlying filesystems to use the current time.  */
      timevals[1].tv_usec = timevals[0].tv_usec = (time_t)-1;
      tvp = timevals;
    }

  port = __file_name_lookup (file, 0, 0);
  if (port == MACH_PORT_NULL)
    return -1;
  err = __file_utimes (port,
		       *(time_value_t *) &tvp[0], *(time_value_t *) &tvp[1]);
  __mach_port_deallocate (__mach_task_self (), port);
  if (err)
    return __hurd_fail (err);
  return 0;
}

weak_alias (__utimes, utimes)
