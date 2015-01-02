/* Copyright (C) 2002-2015 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include "pthreadP.h"


#ifndef NEW_VERNUM
# define NEW_VERNUM GLIBC_2_3_3
#endif


int
__pthread_attr_setstack (attr, stackaddr, stacksize)
     pthread_attr_t *attr;
     void *stackaddr;
     size_t stacksize;
{
  struct pthread_attr *iattr;

  assert (sizeof (*attr) >= sizeof (struct pthread_attr));
  iattr = (struct pthread_attr *) attr;

  /* Catch invalid sizes.  */
  int ret = check_stacksize_attr (stacksize);
  if (ret)
    return ret;

#ifdef EXTRA_PARAM_CHECKS
  EXTRA_PARAM_CHECKS;
#endif

  iattr->stacksize = stacksize;
#if _STACK_GROWS_DOWN
  iattr->stackaddr = (char *) stackaddr + stacksize;
#else
  iattr->stackaddr = (char *) stackaddr;
#endif
  iattr->flags |= ATTR_FLAG_STACKADDR;

  return 0;
}

#if PTHREAD_STACK_MIN == 16384
strong_alias (__pthread_attr_setstack, pthread_attr_setstack)
#else
# include <shlib-compat.h>
versioned_symbol (libpthread, __pthread_attr_setstack, pthread_attr_setstack,
		  NEW_VERNUM);

# if SHLIB_COMPAT(libpthread, GLIBC_2_2, NEW_VERNUM)

int
__old_pthread_attr_setstack (pthread_attr_t *attr, void *stackaddr,
			     size_t stacksize)
{
  struct pthread_attr *iattr;

  assert (sizeof (*attr) >= sizeof (struct pthread_attr));
  iattr = (struct pthread_attr *) attr;

  /* Catch invalid sizes.  */
  if (stacksize < 16384)
    return EINVAL;

#  ifdef EXTRA_PARAM_CHECKS
  EXTRA_PARAM_CHECKS;
#  endif

  iattr->stacksize = stacksize;
#if _STACK_GROWS_DOWN
  iattr->stackaddr = (char *) stackaddr + stacksize;
#else
  iattr->stackaddr = (char *) stackaddr;
#endif
  iattr->flags |= ATTR_FLAG_STACKADDR;

  return 0;
}

compat_symbol (libpthread, __old_pthread_attr_setstack, pthread_attr_setstack,
	       GLIBC_2_2);
# endif

#endif
