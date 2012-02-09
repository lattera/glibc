/* Linux implementation of waitid.
   Copyright (C) 2004 Free Software Foundation, Inc.
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
#include <kernel-features.h>
#include <sysdep.h>


#ifdef __NR_waitid

# if __ASSUME_WAITID_SYSCALL > 0

static inline int
do_waitid (idtype_t idtype, id_t id, siginfo_t *infop, int options)
{
  /* The unused fifth argument is a `struct rusage *' that we could
     pass if we were using waitid to simulate wait3/wait4.  */
  return INLINE_SYSCALL (waitid, 5, idtype, id, infop, options, NULL);
}
# define NO_DO_WAITID

# else

static int do_compat_waitid (idtype_t idtype, id_t id,
			     siginfo_t *infop, int options);
# define DO_WAITID do_compat_waitid

static int
do_waitid (idtype_t idtype, id_t id, siginfo_t *infop, int options)
{
  static int waitid_works;
  if (waitid_works > 0)
    return INLINE_SYSCALL (waitid, 5, idtype, id, infop, options, NULL);
  if (waitid_works == 0)
    {
      int result = INLINE_SYSCALL (waitid, 5,
				   idtype, id, infop, options, NULL);
      if (result < 0 && errno == ENOSYS)
	waitid_works = -1;
      else
	{
	  waitid_works = 1;
	  return result;
	}
    }
  return do_compat_waitid (idtype, id, infop, options);
}

# endif

#endif

#include "sysdeps/posix/waitid.c"
