/* Copyright (C) 1991, 95, 96, 97, 98, 1999 Free Software Foundation, Inc.
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

/* This is a compatibility file.  If we don't build the libc with
   versioning don't compile this file.  */
#if defined PIC && DO_VERSIONING

#include <errno.h>
#include <sys/resource.h>
#include <sys/types.h>

extern int __old_setrlimit (enum __rlimit_resource, const struct rlimit *);

/* Set the soft and hard limits for RESOURCE to *RLIMITS.
   Only the super-user can increase hard limits.
   Return 0 if successful, -1 if not (and sets errno).  */
int
__old_setrlimit64 (enum __rlimit_resource resource,
		   const struct rlimit64 *rlimits)
{
  struct rlimit rlimits32;

  if (rlimits->rlim_cur >= RLIM_INFINITY >> 1)
    rlimits32.rlim_cur = RLIM_INFINITY >> 1;
  else
    rlimits32.rlim_cur = rlimits->rlim_cur;
  if (rlimits->rlim_max >= RLIM_INFINITY >> 1)
    rlimits32.rlim_max = RLIM_INFINITY >> 1;
  else
    rlimits32.rlim_max = rlimits->rlim_max;

  return __old_setrlimit (resource, &rlimits32);
}

symbol_version (__old_setrlimit64, setrlimit64, GLIBC_2.1);

#endif /* PIC && DO_VERSIONING */
