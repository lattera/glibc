/* Copyright (C) 1991, 1995-1999, 2000, 2004 Free Software Foundation, Inc.
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

/* This is a compatibility file.  If we don't build the libc with
   versioning don't compile this file.  */
#include <shlib-compat.h>
#if SHLIB_COMPAT (libc, GLIBC_2_1, GLIBC_2_2)

#include <errno.h>
#include <sys/resource.h>
#include <sys/types.h>

extern int __new_getrlimit (enum __rlimit_resource, struct rlimit *);
extern int __old_getrlimit64 (enum __rlimit_resource resource,
			      struct rlimit64 *rlimits);


/* Put the soft and hard limits for RESOURCE in *RLIMITS.
   Returns 0 if successful, -1 if not (and sets errno).  */
int
attribute_compat_text_section
__old_getrlimit64 (enum __rlimit_resource resource, struct rlimit64 *rlimits)
{
  struct rlimit rlimits32;

  if (__new_getrlimit (resource, &rlimits32) < 0)
    return -1;

  if (rlimits32.rlim_cur == RLIM_INFINITY)
    rlimits->rlim_cur = RLIM64_INFINITY >> 1;
  else
    rlimits->rlim_cur = rlimits32.rlim_cur;
  if (rlimits32.rlim_max == RLIM_INFINITY)
    rlimits->rlim_max = RLIM64_INFINITY >> 1;
  else
    rlimits->rlim_max = rlimits32.rlim_max;

  return 0;
}

compat_symbol (libc, __old_getrlimit64, getrlimit64, GLIBC_2_1);

#endif /* SHLIB_COMPAT */
