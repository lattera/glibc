/* Copyright (C) 2002, 2003, 2005 Free Software Foundation, Inc.
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
#include <stdlib.h>
#include "pthreadP.h"
#include <lowlevellock.h>


int
__pthread_mutex_trylock (mutex)
     pthread_mutex_t *mutex;
{
  pid_t id = THREAD_GETMEM (THREAD_SELF, tid);

  switch (__builtin_expect (mutex->__data.__kind, PTHREAD_MUTEX_TIMED_NP))
    {
      /* Recursive mutex.  */
    case PTHREAD_MUTEX_RECURSIVE_NP:
      /* Check whether we already hold the mutex.  */
      if (mutex->__data.__owner == id)
	{
	  /* Just bump the counter.  */
	  if (__builtin_expect (mutex->__data.__count + 1 == 0, 0))
	    /* Overflow of the counter.  */
	    return EAGAIN;

	  ++mutex->__data.__count;
	  return 0;
	}

      if (lll_mutex_trylock (mutex->__data.__lock) == 0)
	{
	  /* Record the ownership.  */
	  mutex->__data.__owner = id;
	  mutex->__data.__count = 1;
	  ++mutex->__data.__nusers;
	  return 0;
	}
      break;

    case PTHREAD_MUTEX_ERRORCHECK_NP:
      /* Check whether we already hold the mutex.  */
      if (__builtin_expect (mutex->__data.__owner == id, 0))
	return EDEADLK;

      /* FALLTHROUGH */

    case PTHREAD_MUTEX_TIMED_NP:
    case PTHREAD_MUTEX_ADAPTIVE_NP:
      /* Normal mutex.  */
      if (lll_mutex_trylock (mutex->__data.__lock) != 0)
	break;

      /* Record the ownership.  */
      mutex->__data.__owner = id;
      ++mutex->__data.__nusers;

      return 0;


    case PTHREAD_MUTEX_ROBUST_PRIVATE_RECURSIVE_NP:
      /* Check whether we already hold the mutex.  */
      if (abs (mutex->__data.__owner) == id)
	{
	  /* Just bump the counter.  */
	  if (__builtin_expect (mutex->__data.__count + 1 == 0, 0))
	    /* Overflow of the counter.  */
	    return EAGAIN;

	  ++mutex->__data.__count;

	  return 0;
	}

      /* We have to get the mutex.  */
      if (lll_mutex_trylock (mutex->__data.__lock) == 0)
	{
	  mutex->__data.__count = 1;

	  goto robust;
	}

      break;

    case PTHREAD_MUTEX_ROBUST_PRIVATE_ERRORCHECK_NP:
      /* Check whether we already hold the mutex.  */
      if (__builtin_expect (abs (mutex->__data.__owner) == id, 0))
	return EDEADLK;

      /* FALLTHROUGH */

    case PTHREAD_MUTEX_ROBUST_PRIVATE_NP:
    case PTHREAD_MUTEX_ROBUST_PRIVATE_ADAPTIVE_NP:
      if (lll_mutex_trylock (mutex->__data.__lock) != 0)
	break;

    robust:
      if (__builtin_expect (mutex->__data.__owner
			    == PTHREAD_MUTEX_NOTRECOVERABLE, 0))
	{
	  /* This mutex is now not recoverable.  */
	  mutex->__data.__count = 0;
	  lll_mutex_unlock (mutex->__data.__lock);
	  return ENOTRECOVERABLE;
	}

      /* This mutex is either healthy or we can try to recover it.  */
      assert (mutex->__data.__owner == 0
	      || mutex->__data.__owner == PTHREAD_MUTEX_OWNERDEAD);

      /* Record the ownership.  */
      int retval = 0;
      if (__builtin_expect (mutex->__data.__owner
			    == PTHREAD_MUTEX_OWNERDEAD, 0))
	{
	  retval = EOWNERDEAD;
	  /* We signal ownership of a not yet recovered robust
	     mutex by storing the negative thread ID.  */
	  id = -id;
	}

      ENQUEUE_MUTEX (mutex);

      mutex->__data.__owner = id;
      ++mutex->__data.__nusers;

      return retval
;
    default:
      /* Correct code cannot set any other type.  */
      return EINVAL;
    }

  return EBUSY;
}
strong_alias (__pthread_mutex_trylock, pthread_mutex_trylock)
