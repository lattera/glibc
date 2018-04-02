/* Wait on a semaphore with a timeout.  Generic version.
   Copyright (C) 2005-2018 Free Software Foundation, Inc.
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

#include <semaphore.h>
#include <errno.h>
#include <assert.h>

#include <pt-internal.h>

int
__sem_timedwait_internal (sem_t *restrict sem,
			  const struct timespec *restrict timeout)
{
  error_t err;
  int drain;
  struct __pthread *self;

  __pthread_spin_lock (&sem->__lock);
  if (sem->__value > 0)
    /* Successful down.  */
    {
      sem->__value--;
      __pthread_spin_unlock (&sem->__lock);
      return 0;
    }

  if (timeout != NULL && (timeout->tv_nsec < 0 || timeout->tv_nsec >= 1000000000))
    {
      errno = EINVAL;
      return -1;
    }

  /* Add ourselves to the queue.  */
  self = _pthread_self ();

  __pthread_enqueue (&sem->__queue, self);
  __pthread_spin_unlock (&sem->__lock);

  /* Block the thread.  */
  if (timeout != NULL)
    err = __pthread_timedblock (self, timeout, CLOCK_REALTIME);
  else
    {
      err = 0;
      __pthread_block (self);
    }

  __pthread_spin_lock (&sem->__lock);
  if (self->prevp == NULL)
    /* Another thread removed us from the queue, which means a wakeup message
       has been sent.  It was either consumed while we were blocking, or
       queued after we timed out and before we acquired the semaphore lock, in
       which case the message queue must be drained.  */
    drain = err ? 1 : 0;
  else
    {
      /* We're still in the queue.  Noone attempted to wake us up, i.e. we
         timed out.  */
      __pthread_dequeue (self);
      drain = 0;
    }
  __pthread_spin_unlock (&sem->__lock);

  if (drain)
    __pthread_block (self);

  if (err)
    {
      assert (err == ETIMEDOUT);
      errno = err;
      return -1;
    }

  return 0;
}

int
__sem_timedwait (sem_t *restrict sem, const struct timespec *restrict timeout)
{
  return __sem_timedwait_internal (sem, timeout);
}

weak_alias (__sem_timedwait, sem_timedwait);
