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
#include <sys/resource.h>
#include <time.h>
#include <sys/time.h>

#ifdef	__GNUC__
__inline
#endif
static clock_t
DEFUN(timeval_to_clock_t, (tv), CONST struct timeval *tv)
{
  return (clock_t) ((tv->tv_sec * CLK_TCK) +
		    (tv->tv_usec * CLK_TCK / 1000));
}

/* Return the time used by the program so far (user time + system time).  */
clock_t
DEFUN_VOID(clock)
{
  struct rusage usage;

  if (__getrusage(RUSAGE_SELF, &usage) < 0)
    return (clock_t) -1;

  return (timeval_to_clock_t(&usage.ru_stime) +
	  timeval_to_clock_t(&usage.ru_utime)) * CLOCKS_PER_SEC;
}
