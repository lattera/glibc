/* setrlimit function for systems with ulimit system call (SYSV).
   Copyright (C) 1991, 1992, 1996, 1997, 1998 Free Software Foundation, Inc.
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

/* This only implements those functions which are available via ulimit.  */

#include <sys/resource.h>
#include <stddef.h>
#include <errno.h>

/* Set the soft and hard limits for RESOURCE to *RLIMITS.
   Only the super-user can increase hard limits.
   Return 0 if successful, -1 if not (and sets errno).  */
int
setrlimit (resource, rlimits)
     enum __rlimit_resource resource;
     const struct rlimit *rlimits;
{
  if (rlimits == NULL)
    {
      __set_errno (EINVAL);
      return -1;
    }

  switch (resource)
    {
    case RLIMIT_FSIZE:
      return __ulimit (2, rlimits->rlim_cur);

    case RLIMIT_DATA:
    case RLIMIT_CPU:
    case RLIMIT_STACK:
    case RLIMIT_CORE:
    case RLIMIT_RSS:
      __set_errno (ENOSYS);
      return -1;

    default:
      __set_errno (EINVAL);
      return -1;
    }
}
