/* Copyright (C) 2003-2012 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2003.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If
   not, see <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <sysdep.h>
#include <kernel-features.h>
#include "kernel-posix-timers.h"


#ifdef timer_gettime_alias
# define timer_gettime timer_gettime_alias
#endif


int
timer_gettime (timerid, value)
     timer_t timerid;
     struct itimerspec *value;
{
#undef timer_gettime
  struct timer *kt = (struct timer *) timerid;

  /* Delete the kernel timer object.  */
  int res = INLINE_SYSCALL (timer_gettime, 2, kt->ktimerid, value);

  return res;
}
