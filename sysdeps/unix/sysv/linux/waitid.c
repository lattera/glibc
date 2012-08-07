/* Linux implementation of waitid.
   Copyright (C) 2004-2012 Free Software Foundation, Inc.
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

#include <stddef.h>
#include <errno.h>
#include <sys/wait.h>
#include <sysdep.h>

static inline int
do_waitid (idtype_t idtype, id_t id, siginfo_t *infop, int options)
{
  /* The unused fifth argument is a `struct rusage *' that we could
     pass if we were using waitid to simulate wait3/wait4.  */
  return INLINE_SYSCALL (waitid, 5, idtype, id, infop, options, NULL);
}
#define NO_DO_WAITID

#include "sysdeps/posix/waitid.c"
