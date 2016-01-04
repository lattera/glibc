/* Get the current value of a clock.  Stub version.
   Copyright (C) 1999-2016 Free Software Foundation, Inc.
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
#include <time.h>

/* Get current value of CLOCK and store it in TP.  */
int
__clock_gettime (clockid_t clock_id, struct timespec *tp)
{
  __set_errno (ENOSYS);
  return -1;
}
weak_alias (__clock_gettime, clock_gettime)
libc_hidden_def (__clock_gettime)
stub_warning (clock_gettime)
