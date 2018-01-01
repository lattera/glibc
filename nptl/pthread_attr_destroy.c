/* Copyright (C) 2002-2018 Free Software Foundation, Inc.
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

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "pthreadP.h"
#include <shlib-compat.h>

int
__pthread_attr_destroy (pthread_attr_t *attr)
{
  struct pthread_attr *iattr;

  iattr = (struct pthread_attr *) attr;

#if SHLIB_COMPAT(libpthread, GLIBC_2_0, GLIBC_2_1)
  /* In old struct pthread_attr, neither next nor cpuset are
     present.  */
  if (__builtin_expect ((iattr->flags & ATTR_FLAG_OLDATTR), 0) == 0)
#endif
    /* The affinity CPU set might be allocated dynamically.  */
    free (iattr->cpuset);

  return 0;
}
strong_alias (__pthread_attr_destroy, pthread_attr_destroy)
