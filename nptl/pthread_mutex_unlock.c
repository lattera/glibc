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
__pthread_mutex_unlock (mutex)
     pthread_mutex_t *mutex;
{
  switch (__builtin_expect (mutex->__data.__kind, PTHREAD_MUTEX_TIMED_NP))
    {
    case PTHREAD_MUTEX_RECURSIVE_NP:
      /* Recursive mutex.  */
      if (mutex->__data.__owner != THREAD_SELF)
	return EPERM;

      if (--mutex->__data.__count != 0)
	/* We still hold the mutex.  */
	return 0;

      mutex->__data.__owner = NULL;
      break;

    case PTHREAD_MUTEX_ERRORCHECK_NP:
      /* Error checking mutex.  */
      if (mutex->__data.__owner != THREAD_SELF
	  || ! lll_mutex_islocked (mutex->__data.__lock))
	return EPERM;

      mutex->__data.__owner = NULL;
      break;

    default:
      /* Correct code cannot set any other type.  */
    case PTHREAD_MUTEX_TIMED_NP:
    case PTHREAD_MUTEX_ADAPTIVE_NP:
      /* Normal mutex.  Nothing special to do.  */
      break;
    }

  /* Unlock.  */
  lll_mutex_unlock (mutex->__data.__lock);

  return 0;
}
strong_alias (__pthread_mutex_unlock, pthread_mutex_unlock)
INTDEF(__pthread_mutex_unlock)
