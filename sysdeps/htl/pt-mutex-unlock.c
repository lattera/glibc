/* Unlock a mutex.  Generic version.
   Copyright (C) 2000-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library;  if not, see
   <http://www.gnu.org/licenses/>.  */

#include <pthread.h>

#include <pt-internal.h>

#define LOSE do { * (int *) 0 = 0; } while (1)

/* Unlock MUTEX, rescheduling a waiting thread.  */
int
__pthread_mutex_unlock (pthread_mutex_t *mutex)
{
  struct __pthread *wakeup;
  const struct __pthread_mutexattr *attr = mutex->__attr;

  if (attr == __PTHREAD_ERRORCHECK_MUTEXATTR)
    attr = &__pthread_errorcheck_mutexattr;
  if (attr == __PTHREAD_RECURSIVE_MUTEXATTR)
    attr = &__pthread_recursive_mutexattr;

  __pthread_spin_lock (&mutex->__lock);

  if (attr == NULL || attr->__mutex_type == PTHREAD_MUTEX_NORMAL)
    {
#if defined(ALWAYS_TRACK_MUTEX_OWNER)
# ifndef NDEBUG
      if (_pthread_self ())
	{
	  assert (mutex->__owner);
	  assert (mutex->__owner == _pthread_self ());
	  mutex->__owner = NULL;
	}
# endif
#endif
    }
  else
    switch (attr->__mutex_type)
      {
      case PTHREAD_MUTEX_ERRORCHECK:
      case PTHREAD_MUTEX_RECURSIVE:
	if (mutex->__owner != _pthread_self ())
	  {
	    __pthread_spin_unlock (&mutex->__lock);
	    return EPERM;
	  }

	if (attr->__mutex_type == PTHREAD_MUTEX_RECURSIVE)
	  if (--mutex->__locks > 0)
	    {
	      __pthread_spin_unlock (&mutex->__lock);
	      return 0;
	    }

	mutex->__owner = 0;
	break;

      default:
	LOSE;
      }


  if (mutex->__queue == NULL)
    {
      __pthread_spin_unlock (&mutex->__held);
      __pthread_spin_unlock (&mutex->__lock);
      return 0;
    }

  wakeup = mutex->__queue;
  __pthread_dequeue (wakeup);

#ifndef NDEBUG
# if !defined (ALWAYS_TRACK_MUTEX_OWNER)
  if (attr != NULL && attr->__mutex_type != PTHREAD_MUTEX_NORMAL)
# endif
    {
      mutex->__owner = wakeup;
    }
#endif

  /* We do not unlock MUTEX->held: we are transferring the ownership
     to the thread that we are waking up.  */

  __pthread_spin_unlock (&mutex->__lock);
  __pthread_wakeup (wakeup);

  return 0;
}

strong_alias (__pthread_mutex_unlock, _pthread_mutex_unlock);
strong_alias (__pthread_mutex_unlock, pthread_mutex_unlock);
