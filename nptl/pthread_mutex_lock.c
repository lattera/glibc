/* Copyright (C) 2002, 2003, 2004 Free Software Foundation, Inc.
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
#include "pthreadP.h"
#include <lowlevellock.h>


#ifndef LLL_MUTEX_LOCK
# define LLL_MUTEX_LOCK(mutex) lll_mutex_lock (mutex)
# define LLL_MUTEX_TRYLOCK(mutex) lll_mutex_trylock (mutex)
#endif


int
__pthread_mutex_lock (mutex)
     pthread_mutex_t *mutex;
{
  assert (sizeof (mutex->__size) >= sizeof (mutex->__data));

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

      /* We have to get the mutex.  */
      LLL_MUTEX_LOCK (mutex->__data.__lock);

      mutex->__data.__count = 1;
      break;

      /* Error checking mutex.  */
    case PTHREAD_MUTEX_ERRORCHECK_NP:
      /* Check whether we already hold the mutex.  */
      if (mutex->__data.__owner == id)
	return EDEADLK;

      /* FALLTHROUGH */

    default:
      /* Correct code cannot set any other type.  */
    case PTHREAD_MUTEX_TIMED_NP:
    simple:
      /* Normal mutex.  */
      LLL_MUTEX_LOCK (mutex->__data.__lock);
      break;

    case PTHREAD_MUTEX_ADAPTIVE_NP:
      if (! __is_smp)
	goto simple;

      if (LLL_MUTEX_TRYLOCK (mutex->__data.__lock) != 0)
	{
	  int cnt = 0;
	  int max_cnt = MIN (MAX_ADAPTIVE_COUNT,
			     mutex->__data.__spins * 2 + 10);
	  do
	    {
	      if (cnt++ >= max_cnt)
		{
		  LLL_MUTEX_LOCK (mutex->__data.__lock);
		  break;
		}

#ifdef BUSY_WAIT_NOP
	      BUSY_WAIT_NOP;
#endif
	    }
	  while (LLL_MUTEX_TRYLOCK (mutex->__data.__lock) != 0);

	  mutex->__data.__spins += (cnt - mutex->__data.__spins) / 8;
	}
      break;
    }

  /* Record the ownership.  */
  assert (mutex->__data.__owner == 0);
  mutex->__data.__owner = id;
#ifndef NO_INCR
  ++mutex->__data.__nusers;
#endif

  return 0;
}
#ifndef __pthread_mutex_lock
strong_alias (__pthread_mutex_lock, pthread_mutex_lock)
strong_alias (__pthread_mutex_lock, __pthread_mutex_lock_internal)
#endif
