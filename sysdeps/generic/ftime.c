/* Copyright (C) 1994 Free Software Foundation, Inc.
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

#include <sys/timeb.h>
#include <sys/time.h>
#include <errno.h>

int
ftime (timebuf)
     struct timeb *timebuf;
{
  int save = errno;
  struct tm *tp;

  errno = 0;
  if (time (&timebuf->time) == (time_t) -1 && errno != 0)
    return -1;
  timebuf->millitm = 0;
  
  tp = localtime (&timebuf->time);
  if (tp == NULL)
    return -1;

  timebuf->timezone = tp->tm_gmtoff / 60;
  timebuf->dstflag = tp->tm_isdst;

  errno = save;
  return 0;
}
