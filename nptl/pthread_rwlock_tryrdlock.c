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

#include <errno.h>
#include "pthreadP.h"
#include <lowlevellock.h>


int
__pthread_rwlock_tryrdlock (rwlock)
     pthread_rwlock_t *rwlock;
{
  int result = EBUSY;

  lll_mutex_lock (rwlock->__data.__lock);

  if (rwlock->__data.__writer == 0
      && (rwlock->__data.__nr_writers_queued == 0
	  || rwlock->__data.__flags == 0))
    {
      if (__builtin_expect (++rwlock->__data.__nr_readers == 0, 0))
	{
	  --rwlock->__data.__nr_readers;
	  result = EAGAIN;
	}
      else
	result = 0;
    }

  lll_mutex_unlock (rwlock->__data.__lock);

  return result;
}
strong_alias (__pthread_rwlock_tryrdlock, pthread_rwlock_tryrdlock)
