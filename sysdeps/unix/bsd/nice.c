/* Copyright (C) 1992 Free Software Foundation, Inc.
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
#include <unistd.h>
#include <sys/resource.h>

/* Increment the scheduling priority of the calling process by INCR.
   The superuser may use a negative INCR to decrement the priority.  */
int
DEFUN(nice, (incr), int incr)
{
  int save;
  int prio;

  /* -1 is a valid priority, so we use errno to check for an error.  */
  save = errno;
  errno = 0;
  prio = getpriority (PRIO_PROCESS, 0);
  if (prio == -1)
    {
      if (errno != 0)
	return -1;
      else
	errno = save;
    }

  return setpriority (PRIO_PROCESS, 0, prio + incr);
}
