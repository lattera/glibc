/* Copyright (C) 1991, 1992, 1993, 1994, 1995 Free Software Foundation, Inc.
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
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <ansidecl.h>
#include <sys/time.h>
#include <errno.h>
#include <stddef.h>
#include <hurd.h>

/* Change the access time of FILE to TVP[0] and
   the modification time of FILE to TVP[1].  */
int
DEFUN(__utimes, (file, tvp),
      CONST char *file AND struct timeval tvp[2])
{
  error_t err;
  file_t f = __file_name_lookup (file, 0, 0);
  if (f == MACH_PORT_NULL)
    return -1;
  err = __file_utimes (f,
		       *(time_value_t *) &tvp[0], *(time_value_t *) &tvp[1]);
  __mach_port_deallocate (__mach_task_self (), f);
  if (err)
    return __hurd_fail (err);
  return 0;
}

weak_alias (__utimes, utimes)
