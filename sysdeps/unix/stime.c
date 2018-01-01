/* Copyright (C) 1992-2018 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <stddef.h>		/* For NULL.  */
#include <sys/time.h>
#include <time.h>

/* Set the system clock to *WHEN.  */

int
stime (const time_t *when)
{
  struct timeval tv;

  if (when == NULL)
    {
      __set_errno (EINVAL);
      return -1;
    }

  tv.tv_sec = *when;
  tv.tv_usec = 0;
  return __settimeofday (&tv, (struct timezone *) 0);
}
