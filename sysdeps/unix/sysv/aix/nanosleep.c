/* Copyright (C) 2000 Free Software Foundation, Inc.
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

#include <assert.h>
#include <sys/time.h>
#include <sys/types.h>

struct timestruc_t
{
  time_t tv_sec;	/* seconds.  */
  suseconds_t tv_nsec;	/* and nanoseconds.  */
};


extern int _nsleep (struct timestruc_t *rqtp, struct timestruc_t *rmtp);

int
__libc_nanosleep (const struct timespec *req, struct timespec *rem)
{
  assert (sizeof (struct timestruc_t) == sizeof (*req));
  return _nsleep ((struct timestruc_t *) req, (struct timestruc_t *) rem);
}
strong_alias (__libc_nanosleep, __nanosleep)
strong_alias (__libc_nanosleep, nanosleep)
