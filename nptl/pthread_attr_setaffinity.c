/* Copyright (C) 2003-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2003.

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

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <pthreadP.h>
#include <shlib-compat.h>


int
__pthread_attr_setaffinity_new (pthread_attr_t *attr, size_t cpusetsize,
				const cpu_set_t *cpuset)
{
  struct pthread_attr *iattr;

  assert (sizeof (*attr) >= sizeof (struct pthread_attr));
  iattr = (struct pthread_attr *) attr;

  if (cpuset == NULL || cpusetsize == 0)
    {
      free (iattr->cpuset);
      iattr->cpuset = NULL;
      iattr->cpusetsize = 0;
    }
  else
    {
      if (iattr->cpusetsize != cpusetsize)
	{
	  void *newp = (cpu_set_t *) realloc (iattr->cpuset, cpusetsize);
	  if (newp == NULL)
	    return ENOMEM;

	  iattr->cpuset = newp;
	  iattr->cpusetsize = cpusetsize;
	}

      memcpy (iattr->cpuset, cpuset, cpusetsize);
    }

  return 0;
}
versioned_symbol (libpthread, __pthread_attr_setaffinity_new,
		  pthread_attr_setaffinity_np, GLIBC_2_3_4);


#if SHLIB_COMPAT (libpthread, GLIBC_2_3_3, GLIBC_2_3_4)
int
__pthread_attr_setaffinity_old (pthread_attr_t *attr, cpu_set_t *cpuset)
{
  /* The old interface by default assumed a 1024 processor bitmap.  */
  return __pthread_attr_setaffinity_new (attr, 128, cpuset);
}
compat_symbol (libpthread, __pthread_attr_setaffinity_old,
	       pthread_attr_setaffinity_np, GLIBC_2_3_3);
#endif
