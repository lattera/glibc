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

#include <errno.h>
#include "pthreadP.h"
#include <lowlevellock.h>


int
pthread_mutex_timedlock (mutex, abstime)
     pthread_mutex_t *mutex;
     const struct timespec *abstime;
{
  pid_t id = THREAD_GETMEM (THREAD_SELF, tid);
  int result = 0;

  /* We must not check ABSTIME here.  If the thread does not block
     abstime must not be checked for a valid value.  */

  switch (mutex->__data.__kind)
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

	  goto out;
	}
      else
	{
	  /* We have to get the mutex.  */
	  result = lll_mutex_timedlock (mutex->__data.__lock, abstime);

	  if (result != 0)
	    goto out;

	  /* Only locked once so far.  */
	  mutex->__data.__count = 1;
	}
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
      result = lll_mutex_timedlock (mutex->__data.__lock, abstime);
      break;

    case PTHREAD_MUTEX_ADAPTIVE_NP:
      if (! __is_smp)
	goto simple;

      if (lll_mutex_trylock (mutex->__data.__lock) != 0)
	{
	  int cnt = 0;
	  int max_cnt = MIN (MAX_ADAPTIVE_COUNT,
			     mutex->__data.__spins * 2 + 10);
	  do
	    {
	      if (cnt++ >= max_cnt)
		{
		  result = lll_mutex_timedlock (mutex->__data.__lock, abstime);
		  break;
		}

#ifdef BUSY_WAIT_NOP
	      BUSY_WAIT_NOP;
#endif
	    }
	  while (lll_mutex_trylock (mutex->__data.__lock) != 0);

	  mutex->__data.__spins += (cnt - mutex->__data.__spins) / 8;
	}
      break;
    }

  if (result == 0)
    {
      /* Record the ownership.  */
      mutex->__data.__owner = id;
      ++mutex->__data.__nusers;
    }

 out:
  return result;
}
