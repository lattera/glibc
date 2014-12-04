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

#include <errno.h>
#include "pthreadP.h"
#include <lowlevellock.h>
#include <kernel-features.h>


static const struct pthread_barrierattr default_barrierattr =
  {
    .pshared = PTHREAD_PROCESS_PRIVATE
  };


int
__pthread_barrier_init (barrier, attr, count)
     pthread_barrier_t *barrier;
     const pthread_barrierattr_t *attr;
     unsigned int count;
{
  struct pthread_barrier *ibarrier;

  /* XXX EINVAL is not specified by POSIX as a possible error code.  */
  if (__glibc_unlikely (count == 0))
    return EINVAL;

  const struct pthread_barrierattr *iattr
    = (attr != NULL
       ? iattr = (struct pthread_barrierattr *) attr
       : &default_barrierattr);

  ibarrier = (struct pthread_barrier *) barrier;

  /* Initialize the individual fields.  */
  ibarrier->lock = LLL_LOCK_INITIALIZER;
  ibarrier->left = count;
  ibarrier->init_count = count;
  ibarrier->curr_event = 0;

  /* XXX Don't use FUTEX_SHARED or FUTEX_PRIVATE as long as there are still
     assembly implementations that expect the value determined below.  */
  ibarrier->private = (iattr->pshared != PTHREAD_PROCESS_PRIVATE
		       ? 0 : FUTEX_PRIVATE_FLAG);

  return 0;
}
weak_alias (__pthread_barrier_init, pthread_barrier_init)
