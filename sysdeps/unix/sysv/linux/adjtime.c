/* Copyright (C) 1995-2018 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <limits.h>
#include <sys/time.h>
#include <sys/timex.h>

#define MAX_SEC	(INT_MAX / 1000000L - 2)
#define MIN_SEC	(INT_MIN / 1000000L + 2)

#ifndef MOD_OFFSET
#define modes mode
#endif

#ifndef TIMEVAL
#define TIMEVAL timeval
#endif

#ifndef TIMEX
#define TIMEX timex
#endif

#ifndef ADJTIME
#define ADJTIME __adjtime
#endif

#ifndef ADJTIMEX
#define NO_LOCAL_ADJTIME
#define ADJTIMEX(x) __adjtimex (x)
#endif

#ifndef LINKAGE
#define LINKAGE
#endif

LINKAGE int
ADJTIME (const struct TIMEVAL *itv, struct TIMEVAL *otv)
{
  struct TIMEX tntx;

  if (itv)
    {
      struct TIMEVAL tmp;

      /* We will do some check here. */
      tmp.tv_sec = itv->tv_sec + itv->tv_usec / 1000000L;
      tmp.tv_usec = itv->tv_usec % 1000000L;
      if (tmp.tv_sec > MAX_SEC || tmp.tv_sec < MIN_SEC)
	return INLINE_SYSCALL_ERROR_RETURN_VALUE (EINVAL);
      tntx.offset = tmp.tv_usec + tmp.tv_sec * 1000000L;
      tntx.modes = ADJ_OFFSET_SINGLESHOT;
    }
  else
    tntx.modes = ADJ_OFFSET_SS_READ;

  if (__glibc_unlikely (ADJTIMEX (&tntx) < 0))
    return -1;

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

#ifdef NO_LOCAL_ADJTIME
weak_alias (__adjtime, adjtime)
#endif
