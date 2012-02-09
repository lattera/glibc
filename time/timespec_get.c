/* Copyright (C) 2011 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <time.h>


/* Set TS to calendar time based in time base BASE.  */
int
timespec_get (ts, base)
     struct timespec *ts;
     int base;
{
  switch (base)
    {
    case TIME_UTC:
      /* Not supported.  */
      return 0;

    default:
      return 0;
    }

  return base;
}
stub_warning (timespec_get)
