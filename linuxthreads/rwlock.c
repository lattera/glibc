/* Read-write lock implementation.
   Copyright (C) 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Xavier Leroy <Xavier.Leroy@inria.fr>
   and Ulrich Drepper <drepper@cygnus.com>, 1998.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <errno.h>
#include <pthread.h>
#include "internals.h"
#include "queue.h"
#include "restart.h"
#include "spinlock.h"

int
pthread_rwlock_init (pthread_rwlock_t *rwlock,
		     const pthread_rwlockattr_t *attr)
{
  __pthread_init_lock(&rwlock->rw_lock);
  rwlock->rw_readers = 0;
  rwlock->rw_writer = NULL;
  rwlock->rw_read_waiting = NULL;
  rwlock->rw_write_waiting = NULL;

  if (attr == NULL)
    {
      rwlock->rw_kind = PTHREAD_RWLOCK_DEFAULT_NP;
      rwlock->rw_pshared = PTHREAD_PROCESS_PRIVATE;
    }
  else
    {
      rwlock->rw_kind = attr->lockkind;
      rwlock->rw_pshared = attr->pshared;
    }

  return 0;
}


int
pthread_rwlock_destroy (pthread_rwlock_t *rwlock)
{
  int readers;
  _pthread_descr writer;

  __pthread_lock (&rwlock->rw_lock);
  readers = rwlock->rw_readers;
  writer = rwlock->rw_writer;
  __pthread_unlock (&rwlock->rw_lock);

  if (readers > 0 || writer != NULL)
    return EBUSY;

  return 0;
}


int
pthread_rwlock_rdlock (pthread_rwlock_t *rwlock)
{
  pthread_descr self;

  while (1)
    {
      __pthread_lock (&rwlock->rw_lock);
      if (rwlock->rw_writer == NULL
	  || (rwlock->rw_kind == PTHREAD_RWLOCK_PREFER_READER_NP
	      && rwlock->rw_readers != 0))
	/* We can add a reader lock.  */
	break;

      /* Suspend ourselves, then try again */
      self = thread_self ();
      enqueue (&rwlock->rw_read_waiting, self);
      __pthread_unlock (&rwlock->rw_lock);
      suspend (self); /* This is not a cancellation point */
    }

  ++rwlock->rw_readers;
  __pthread_unlock (&rwlock->rw_lock);

  return 0;
}


int
pthread_rwlock_tryrdlock (pthread_rwlock_t *rwlock)
{
  int result = EBUSY;

  __pthread_lock (&rwlock->rw_lock);
  if (rwlock->rw_writer == NULL
      || (rwlock->rw_kind == PTHREAD_RWLOCK_PREFER_READER_NP
	  && rwlock->rw_readers != 0))
    {
      ++rwlock->rw_readers;
      result = 0;
    }
  __pthread_unlock (&rwlock->rw_lock);

  return result;
}


int
pthread_rwlock_wrlock (pthread_rwlock_t *rwlock)
{
  pthread_descr self = thread_self ();

  while(1)
    {
      __pthread_lock (&rwlock->rw_lock);
      if (rwlock->rw_readers == 0 && rwlock->rw_writer == NULL)
	{
	  rwlock->rw_writer = self;
	  __pthread_unlock (&rwlock->rw_lock);
	  return 0;
	}

      /* Suspend ourselves, then try again */
      enqueue (&rwlock->rw_write_waiting, self);
      __pthread_unlock (&rwlock->rw_lock);
      suspend (self); /* This is not a cancellation point */
    }
}


int
pthread_rwlock_trywrlock (pthread_rwlock_t *rwlock)
{
  int result = EBUSY;

  __pthread_lock (&rwlock->rw_lock);
  if (rwlock->rw_readers == 0 && rwlock->rw_writer == NULL)
    {
      rwlock->rw_writer = thread_self ();
      result = 0;
    }
  __pthread_unlock (&rwlock->rw_lock);

  return result;
}


int
pthread_rwlock_unlock (pthread_rwlock_t *rwlock)
{
  pthread_descr torestart;
  pthread_descr th;

  __pthread_lock (&rwlock->rw_lock);
  if (rwlock->rw_writer != NULL)
    {
      /* Unlocking a write lock.  */
      if (rwlock->rw_writer != thread_self ())
	{
	  __pthread_unlock (&rwlock->rw_lock);
	  return EPERM;
	}
      rwlock->rw_writer = NULL;

      if (rwlock->rw_kind == PTHREAD_RWLOCK_PREFER_READER_NP
	  || (th = dequeue (&rwlock->rw_write_waiting)) == NULL)
	{
	  /* Restart all waiting readers.  */
	  torestart = rwlock->rw_read_waiting;
	  rwlock->rw_read_waiting = NULL;
	  __pthread_unlock (&rwlock->rw_lock);
	  while ((th = dequeue (&torestart)) != NULL)
	    restart (th);
	}
      else
	{
	  /* Restart one waiting writer.  */
	  __pthread_unlock (&rwlock->rw_lock);
	  restart (th);
	}
    }
  else
    {
      /* Unlocking a read lock.  */
      if (rwlock->rw_readers == 0)
	{
	  __pthread_unlock (&rwlock->rw_lock);
	  return EPERM;
	}

      --rwlock->rw_readers;
      if (rwlock->rw_readers == 0)
	/* Restart one waiting writer, if any.  */
	th = dequeue (&rwlock->rw_write_waiting);
      else
	th = NULL;

      __pthread_unlock (&rwlock->rw_lock);
      if (th != NULL)
	restart (th);
    }

  return 0;
}



int
pthread_rwlockattr_init (pthread_rwlockattr_t *attr)
{
  attr->lockkind = 0;
  attr->pshared = 0;

  return 0;
}


int
pthread_rwlockattr_destroy (pthread_rwlockattr_t *attr)
{
  return 0;
}


int
pthread_rwlockattr_getpshared (const pthread_rwlockattr_t *attr, int *pshared)
{
  *pshared = attr->pshared;
  return 0;
}


int
pthread_rwlockattr_setpshared (pthread_rwlockattr_t *attr, int pshared)
{
  if (pshared != PTHREAD_PROCESS_PRIVATE && pshared != PTHREAD_PROCESS_SHARED)
    return EINVAL;

  attr->pshared = pshared;

  return 0;
}


int
pthread_rwlockattr_getkind_np (const pthread_rwlockattr_t *attr, int *pref)
{
  *pref = attr->lockkind;
  return 0;
}


int
pthread_rwlockattr_setkind_np (pthread_rwlockattr_t *attr, int pref)
{
  if (pref != PTHREAD_RWLOCK_PREFER_READER_NP
      && pref != PTHREAD_RWLOCK_PREFER_WRITER_NP
      && pref != PTHREAD_RWLOCK_DEFAULT_NP)
    return EINVAL;

  attr->lockkind = pref;

  return 0;
}
