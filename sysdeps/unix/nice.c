/* Copyright (C) 1992, 1996, 1997, 2001, 2002, 2006
   Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <errno.h>
#include <unistd.h>
#include <sys/resource.h>

/* Increment the scheduling priority of the calling process by INCR.
   The superuser may use a negative INCR to decrement the priority.  */
int
nice (int incr)
{
  int save;
  int prio;
  int result;

  /* -1 is a valid priority, so we use errno to check for an error.  */
  save = errno;
  __set_errno (0);
  prio = getpriority (PRIO_PROCESS, 0);
  if (prio == -1)
    {
      if (errno != 0)
	return -1;
      else
	__set_errno (save);
    }

  result = setpriority (PRIO_PROCESS, 0, prio + incr);
  if (result == -1)
    {
      if (errno == EACCES)
	errno = EPERM;
      return -1;
    }
  return getpriority (PRIO_PROCESS, 0);
}
