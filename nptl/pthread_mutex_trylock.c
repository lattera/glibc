/* Copyright (C) 2002, 2003, 2005, 2006 Free Software Foundation, Inc.
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
  int oldval;
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
    case PTHREAD_MUTEX_ROBUST_PRIVATE_ERRORCHECK_NP:
    case PTHREAD_MUTEX_ROBUST_PRIVATE_NP:
    case PTHREAD_MUTEX_ROBUST_PRIVATE_ADAPTIVE_NP:
      oldval = mutex->__data.__lock;
      do
	{
	  if ((oldval & FUTEX_OWNER_DIED) != 0)
	    {
	      /* The previous owner died.  Try locking the mutex.  */
	      int newval;
	      while ((newval
		      = atomic_compare_and_exchange_val_acq (&mutex->__data.__lock,
							     id, oldval))
		     != oldval)
		{
		  if ((newval & FUTEX_OWNER_DIED) == 0)
		    goto normal;
		  oldval = newval;
		}

	      /* We got the mutex.  */
	      mutex->__data.__count = 1;
	      /* But it is inconsistent unless marked otherwise.  */
	      mutex->__data.__owner = PTHREAD_MUTEX_INCONSISTENT;

	      ENQUEUE_MUTEX (mutex);

	      /* Note that we deliberately exist here.  If we fall
		 through to the end of the function __nusers would be
		 incremented which is not correct because the old
		 owner has to be discounted.  */
	      return EOWNERDEAD;
	    }

	normal:
	  /* Check whether we already hold the mutex.  */
	  if (__builtin_expect ((mutex->__data.__lock & FUTEX_TID_MASK)
				== id, 0))
	    {
	      if (mutex->__data.__kind
		  == PTHREAD_MUTEX_ROBUST_PRIVATE_ERRORCHECK_NP)
		return EDEADLK;

	      if (mutex->__data.__kind
		  == PTHREAD_MUTEX_ROBUST_PRIVATE_RECURSIVE_NP)
		{
		  /* Just bump the counter.  */
		  if (__builtin_expect (mutex->__data.__count + 1 == 0, 0))
		    /* Overflow of the counter.  */
		    return EAGAIN;

		  ++mutex->__data.__count;

		  return 0;
		}
	    }

	  oldval = lll_robust_mutex_trylock (mutex->__data.__lock, id);
	  if (oldval != 0 && (oldval & FUTEX_OWNER_DIED) == 0)
	    return EBUSY;

	robust:
	  if (__builtin_expect (mutex->__data.__owner
				== PTHREAD_MUTEX_NOTRECOVERABLE, 0))
	    {
	      /* This mutex is now not recoverable.  */
	      mutex->__data.__count = 0;
	      if (oldval == id)
		lll_mutex_unlock (mutex->__data.__lock);
	      return ENOTRECOVERABLE;
	    }
	}
      while ((oldval & FUTEX_OWNER_DIED) != 0);

      ENQUEUE_MUTEX (mutex);

      mutex->__data.__owner = id;
      ++mutex->__data.__nusers;
      mutex->__data.__count = 1;

      return 0;

    default:
      /* Correct code cannot set any other type.  */
      return EINVAL;
    }

  return EBUSY;
}
strong_alias (__pthread_mutex_trylock, pthread_mutex_trylock)
