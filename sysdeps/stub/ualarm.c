/* Copyright (C) 1991 Free Software Foundation, Inc.
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

/* Set an alarm to go off (generating a SIGALRM signal) in VALUE microseconds.
   If INTERVAL is nonzero, when the alarm goes off, the timer is reset to go
   off every INTERVAL microseconds thereafter.

   Returns the number of microseconds remaining before the alarm.  */
unsigned int
DEFUN(ualarm, (value, interval),
      unsigned int value AND unsigned int interval)
{
  errno = ENOSYS;
  return -1;
}
