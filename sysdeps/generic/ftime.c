/* Copyright (C) 1994, 1996, 1997, 2001 Free Software Foundation, Inc.
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
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <time.h>
#include <sys/timeb.h>
#include <sys/time.h>
#include <errno.h>

int
ftime (timebuf)
     struct timeb *timebuf;
{
  int save = errno;
  struct tm tp;

  __set_errno (0);
  if (time (&timebuf->time) == (time_t) -1 && errno != 0)
    return -1;
  timebuf->millitm = 0;

  if (__localtime_r (&timebuf->time, &tp) == NULL)
    return -1;

  timebuf->timezone = tp.tm_gmtoff / 60;
  timebuf->dstflag = tp.tm_isdst;

  __set_errno (save);
  return 0;
}
