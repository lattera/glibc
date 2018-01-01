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

#include "pthreadP.h"
#include <string.h>


static const struct pthread_rwlockattr default_rwlockattr =
  {
    .lockkind = PTHREAD_RWLOCK_DEFAULT_NP,
    .pshared = PTHREAD_PROCESS_PRIVATE
  };


/* See pthread_rwlock_common.c.  */
int
__pthread_rwlock_init (pthread_rwlock_t *rwlock,
		       const pthread_rwlockattr_t *attr)
{
  ASSERT_TYPE_SIZE (pthread_rwlock_t, __SIZEOF_PTHREAD_RWLOCK_T);

  const struct pthread_rwlockattr *iattr;

  iattr = ((const struct pthread_rwlockattr *) attr) ?: &default_rwlockattr;

  memset (rwlock, '\0', sizeof (*rwlock));

  rwlock->__data.__flags = iattr->lockkind;

  /* The value of __SHARED in a private rwlock must be zero.  */
  rwlock->__data.__shared = (iattr->pshared != PTHREAD_PROCESS_PRIVATE);

  return 0;
}
strong_alias (__pthread_rwlock_init, pthread_rwlock_init)
