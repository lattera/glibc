/* Copyright (C) 1991, 1992, 1995 Free Software Foundation, Inc.
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
#include <errno.h>
#include <sys/time.h>
#include <mach.h>

/* Get the current time of day and timezone information,
   putting it into *TV and *TZ.  If TZ is NULL, *TZ is not filled.
   Returns 0 on success, -1 on errors.  */
int
DEFUN(__gettimeofday, (tv, tz),
      struct timeval *tv AND struct timezone *tz)
{
  kern_return_t err;

  if (tz != NULL)
    {
      errno = ENOSYS;
      return -1;
    }

  if (err = __host_get_time (__mach_host_self (), (time_value_t *) tv))
    {
      errno = err;
      return -1;
    }
  return 0;
}

weak_alias (__gettimeofday, gettimeofday)
