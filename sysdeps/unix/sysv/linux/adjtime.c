/* Copyright (C) 1995, 1996, 1997, 1998 Free Software Foundation, Inc.
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
#include <limits.h>
#include <sys/time.h>
#include <sys/timex.h>

#define MAX_SEC	(INT_MAX / 1000000L - 2)
#define MIN_SEC	(INT_MIN / 1000000L + 2)

int
__adjtime (itv, otv)
     const struct timeval *itv;
     struct timeval *otv;
{
  struct timex tntx;

  if (itv)
    {
      struct timeval tmp;

      /* We will do some check here. */
      tmp.tv_sec = itv->tv_sec + itv->tv_usec / 1000000L;
      tmp.tv_usec = itv->tv_usec % 1000000L;
      if (tmp.tv_sec > MAX_SEC || tmp.tv_sec < MIN_SEC)
	{
	  __set_errno (EINVAL);
	  return -1;
	}
      tntx.offset = tmp.tv_usec + tmp.tv_sec * 1000000L;
      tntx.modes = ADJ_OFFSET_SINGLESHOT;
    }
  else
    tntx.modes = 0;

  if (__adjtimex (&tntx) < 0) return -1;

  if (otv)
    {
      if (tntx.offset < 0)
	{
	  otv->tv_usec = -(-tntx.offset % 1000000);
	  otv->tv_sec  = -(-tntx.offset / 1000000);
	}
      else
	{
	  otv->tv_usec = tntx.offset % 1000000;
	  otv->tv_sec  = tntx.offset / 1000000;
	}
    }
  return 0;
}

weak_alias (__adjtime, adjtime)
