/* Copyright (C) 1991, 1994 Free Software Foundation, Inc.
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
#include <sysdep.h>
#include <errno.h>
#include <utime.h>
#include <time.h>
#include <sys/types.h>
#include <sys/time.h>


/* Set the access and modification times of FILE to those given in TIMES.
   If TIMES is NULL, set them to the current time.  */
int
DEFUN(utime, (file, times), CONST char *file AND CONST struct utimbuf *times)
{
  struct timeval timevals[2];

  if (times != NULL)
    {
      timevals[0].tv_sec = (long int) times->actime;
      timevals[0].tv_usec = 0L;
      timevals[1].tv_sec = (long int) times->modtime;
      timevals[1].tv_usec = 0L;
    }
  else
    {
      if (__gettimeofday (&timevals[0], NULL) < 0)
	return -1;
      timevals[1] = timevals[0];
    }

  return __utimes (file, timevals);
}
