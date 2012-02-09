/* Determine realtime clock frequency.
   Copyright (C) 2003, 2004, 2006 Free Software Foundation, Inc.
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

#include <sys/time.h>
#include <libc-internal.h>
#include <kernel-features.h>
#include <ldsodefs.h>


int
__profile_frequency (void)
{
#ifdef __ASSUME_AT_CLKTCK
  return GLRO(dl_clktck);
#else
  if (GLRO(dl_clktck) != 0)
    return GLRO(dl_clktck);

  struct itimerval tim;

  tim.it_interval.tv_sec = 0;
  tim.it_interval.tv_usec = 1;
  tim.it_value.tv_sec = 0;
  tim.it_value.tv_usec = 0;

  __setitimer (ITIMER_REAL, &tim, 0);
  __setitimer (ITIMER_REAL, 0, &tim);

  if (tim.it_interval.tv_usec < 2)
    return 0;

  return 1000000 / tim.it_interval.tv_usec;
#endif
}
libc_hidden_def (__profile_frequency)
