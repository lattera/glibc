/* Copyright (C) 1999, 2000, 2001 Free Software Foundation, Inc.
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

#include <errno.h>
#include <time.h>
#include <sys/time.h>


#ifndef EXTRA_CLOCK_CASES
# define EXTRA_CLOCK_CASES
#endif

/* Get current value of CLOCK and store it in TP.  */
int
clock_gettime (clockid_t clock_id, struct timespec *tp)
{
  struct timeval tv;
  int retval = -1;

  switch (clock_id)
    {
    case CLOCK_REALTIME:
      retval = gettimeofday (&tv, NULL);
      if (retval == 0)
	/* Convert into `timespec'.  */
	TIMEVAL_TO_TIMESPEC (&tv, tp);
      break;

      EXTRA_CLOCK_CASES

    default:
      __set_errno (EINVAL);
      break;
    }

  return retval;
}
