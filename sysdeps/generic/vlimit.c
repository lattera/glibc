/* Copyright (C) 1991, 1996, 1997, 1998, 2000 Free Software Foundation, Inc.
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

/* This is generic in the sense that it will work with the BSD, SYSV,
   or stub versions of getrlimit.  Separate versions could be written
   for efficiency, but it's probably not worth it.  */

#include <sys/vlimit.h>
#include <sys/resource.h>
#include <errno.h>

/* Set the soft limit for RESOURCE to be VALUE.
   Returns 0 for success, -1 for failure.  */
int
vlimit (resource, value)
     enum __vlimit_resource resource;
     int value;
{
  if (resource >= LIM_CPU && resource <= LIM_MAXRSS)
    {
      /* The rlimit codes happen to each be one less
	 than the corresponding vlimit codes.  */
      enum __rlimit_resource rlimit_res =
	(enum __rlimit_resource) ((int) resource - 1);
      struct rlimit lims;

      if (__getrlimit (rlimit_res, &lims) < 0)
	return -1;

      lims.rlim_cur = value;
      return __setrlimit (rlimit_res, &lims);
    }

  __set_errno (EINVAL);
  return -1;
}
