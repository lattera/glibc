/* Spin locks.
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
#include <sched.h>

/* The default for single processor machines; don't spin, it's
   pointless.  */
#ifndef __PTHREAD_SPIN_COUNT
# define __PTHREAD_SPIN_COUNT 1
#endif

/* The number of times to spin while trying to lock a spin lock object
   before yielding the processor. */
int __pthread_spin_count = __PTHREAD_SPIN_COUNT;


/* Lock the spin lock object LOCK.  If the lock is held by another
   thread spin until it becomes available.  */
int
_pthread_spin_lock (__pthread_spinlock_t *lock)
{
  int i;

  while (1)
    {
      for (i = 0; i < __pthread_spin_count; i++)
	{
	  if (__pthread_spin_trylock (lock) == 0)
	    return 0;
	}

      __sched_yield ();
    }
}
