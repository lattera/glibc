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

#include "pthreadP.h"


static const struct pthread_rwlockattr default_attr =
  {
    .lockkind = PTHREAD_RWLOCK_DEFAULT_NP,
    .pshared = PTHREAD_PROCESS_PRIVATE
  };


int
__pthread_rwlock_init (rwlock, attr)
     pthread_rwlock_t *rwlock;
     const pthread_rwlockattr_t *attr;
{
  const struct pthread_rwlockattr *iattr;

  iattr = ((const struct pthread_rwlockattr *) attr) ?: &default_attr;

  rwlock->__data.__lock = 0;
  rwlock->__data.__flags
    = iattr->lockkind == PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP;
  rwlock->__data.__nr_readers = 0;
  rwlock->__data.__writer = 0;
  rwlock->__data.__readers_wakeup = 0;
  rwlock->__data.__writer_wakeup = 0;
  rwlock->__data.__nr_readers_queued = 0;
  rwlock->__data.__nr_writers_queued = 0;

  return 0;
}
strong_alias (__pthread_rwlock_init, pthread_rwlock_init)
