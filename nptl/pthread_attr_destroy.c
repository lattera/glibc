/* Copyright (C) 2002 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2002.

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
#include <string.h>
#include <unistd.h>
#include "pthreadP.h"


int
__pthread_attr_destroy (attr)
     pthread_attr_t *attr;
{
  /* Enqueue the attributes to the list of all known variables.  */
  if (DEBUGGING_P)
    {
      struct pthread_attr *iattr;
      struct pthread_attr *prevp = NULL;
      struct pthread_attr *runp;

      assert (sizeof (*attr) >= sizeof (struct pthread_attr));
      iattr = (struct pthread_attr *) attr;

      lll_lock (__attr_list_lock);

      runp = __attr_list;
      while (runp != NULL && runp != iattr)
	{
	  prevp = runp;
	  runp = runp->next;
	}

      if (runp != NULL)
	{
	  if (prevp == NULL)
	    __attr_list = iattr->next;
	  else
	    prevp->next = iattr->next;
	}

      lll_unlock (__attr_list_lock);

      if (runp == NULL)
	/* Not a valid attribute.  */
	return EINVAL;
    }

  return 0;
}
strong_alias (__pthread_attr_destroy, pthread_attr_destroy)
