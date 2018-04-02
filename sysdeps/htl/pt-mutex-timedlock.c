/* Lock a mutex with a timeout.  Generic version.
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
#include <assert.h>

#include <pt-internal.h>

#define LOSE do { * (int *) 0 = 0; } while (1)

/* Try to lock MUTEX, block until *ABSTIME if it is already held.  As
   a GNU extension, if TIMESPEC is NULL then wait forever.  */
int
__pthread_mutex_timedlock_internal (struct __pthread_mutex *mutex,
				    const struct timespec *abstime)
{
  error_t err;
  int drain;
  struct __pthread *self;
  const struct __pthread_mutexattr *attr = mutex->__attr;

  if (attr == __PTHREAD_ERRORCHECK_MUTEXATTR)
    attr = &__pthread_errorcheck_mutexattr;
  if (attr == __PTHREAD_RECURSIVE_MUTEXATTR)
    attr = &__pthread_recursive_mutexattr;

  __pthread_spin_lock (&mutex->__lock);
  if (__pthread_spin_trylock (&mutex->__held) == 0)
    /* Successfully acquired the lock.  */
    {
#ifdef ALWAYS_TRACK_MUTEX_OWNER
# ifndef NDEBUG
      self = _pthread_self ();
      if (self != NULL)
	/* The main thread may take a lock before the library is fully
	   initialized, in particular, before the main thread has a
	   TCB.  */
	{
	  assert (mutex->__owner == NULL);
	  mutex->__owner = _pthread_self ();
	}
# endif
#endif

      if (attr != NULL)
	switch (attr->__mutex_type)
	  {
	  case PTHREAD_MUTEX_NORMAL:
	    break;

	  case PTHREAD_MUTEX_RECURSIVE:
	    mutex->__locks = 1;
	  case PTHREAD_MUTEX_ERRORCHECK:
	    mutex->__owner = _pthread_self ();
	    break;

	  default:
	    LOSE;
	  }

      __pthread_spin_unlock (&mutex->__lock);
      return 0;
    }

  /* The lock is busy.  */

  self = _pthread_self ();
  assert (self);

  if (attr == NULL || attr->__mutex_type == PTHREAD_MUTEX_NORMAL)
    {
#if defined(ALWAYS_TRACK_MUTEX_OWNER)
      assert (mutex->__owner != self);
#endif
    }
  else
    {
      switch (attr->__mutex_type)
	{
	case PTHREAD_MUTEX_ERRORCHECK:
	  if (mutex->__owner == self)
	    {
	      __pthread_spin_unlock (&mutex->__lock);
	      return EDEADLK;
	    }
	  break;

	case PTHREAD_MUTEX_RECURSIVE:
	  if (mutex->__owner == self)
	    {
	      mutex->__locks++;
	      __pthread_spin_unlock (&mutex->__lock);
	      return 0;
	    }
	  break;

	default:
	  LOSE;
	}
    }

#if !defined(ALWAYS_TRACK_MUTEX_OWNER)
  if (attr != NULL && attr->__mutex_type != PTHREAD_MUTEX_NORMAL)
#endif
    assert (mutex->__owner);

  if (abstime != NULL && (abstime->tv_nsec < 0 || abstime->tv_nsec >= 1000000000))
    return EINVAL;

  /* Add ourselves to the queue.  */
  __pthread_enqueue (&mutex->__queue, self);
  __pthread_spin_unlock (&mutex->__lock);

  /* Block the thread.  */
  if (abstime != NULL)
    err = __pthread_timedblock (self, abstime, CLOCK_REALTIME);
  else
    {
      err = 0;
      __pthread_block (self);
    }

  __pthread_spin_lock (&mutex->__lock);
  if (self->prevp == NULL)
    /* Another thread removed us from the queue, which means a wakeup message
       has been sent.  It was either consumed while we were blocking, or
       queued after we timed out and before we acquired the mutex lock, in
       which case the message queue must be drained.  */
    drain = err ? 1 : 0;
  else
    {
      /* We're still in the queue.  Noone attempted to wake us up, i.e. we
         timed out.  */
      __pthread_dequeue (self);
      drain = 0;
    }
  __pthread_spin_unlock (&mutex->__lock);

  if (drain)
    __pthread_block (self);

  if (err)
    {
      assert (err == ETIMEDOUT);
      return err;
    }

#if !defined(ALWAYS_TRACK_MUTEX_OWNER)
  if (attr != NULL && attr->__mutex_type != PTHREAD_MUTEX_NORMAL)
#endif
    {
      assert (mutex->__owner == self);
    }

  if (attr != NULL)
    switch (attr->__mutex_type)
      {
      case PTHREAD_MUTEX_NORMAL:
	break;

      case PTHREAD_MUTEX_RECURSIVE:
	assert (mutex->__locks == 0);
	mutex->__locks = 1;
      case PTHREAD_MUTEX_ERRORCHECK:
	mutex->__owner = self;
	break;

      default:
	LOSE;
      }

  return 0;
}

int
pthread_mutex_timedlock (struct __pthread_mutex *mutex,
			 const struct timespec *abstime)
{
  return __pthread_mutex_timedlock_internal (mutex, abstime);
}
