/* Copyright (C) 2003 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <pthreadP.h>


int
pthread_attr_setaffinity_np (attr, cpuset)
     pthread_attr_t *attr;
     const cpu_set_t *cpuset;
{
  struct pthread_attr *iattr;

  assert (sizeof (*attr) >= sizeof (struct pthread_attr));
  iattr = (struct pthread_attr *) attr;

  if (cpuset == NULL)
    {
      free (iattr->cpuset);
      iattr->cpuset = NULL;
    }
  else
    {
      if (iattr->cpuset == NULL)
	{
	  iattr->cpuset = (cpu_set_t *) malloc (sizeof (cpu_set_t));
	  if (iattr->cpuset == NULL)
	    return ENOMEM;
	}

      memcpy (iattr->cpuset, cpuset, sizeof (cpu_set_t));
    }

  return 0;
}
