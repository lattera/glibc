/* sleep - Implementation of the POSIX sleep function using nanosleep.
Copyright (C) 1996 Free Software Foundation, Inc.
This file is part of the GNU C Library.
Contributed by Ulrich Drepper <drepper@cygnus.com>, 1996.

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
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

#include <time.h>
#include <unistd.h>

unsigned int
sleep (unsigned int seconds)
{
  struct timespec ts = { tv_sec: (long int) seconds, tv_nsec: 0 };
  unsigned int result;

  if (__nanosleep (&ts, &ts) == 0)
    result = 0;
  else
    /* Round remaining time.  */
    result = (unsigned int) ts.tv_sec + (ts.tv_nsec >= 500000000L);

  return result;
}
