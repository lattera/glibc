/* Linuxthreads - a simple clone()-based implementation of Posix        */
/* threads for Linux.                                                   */
/* Copyright (C) 1996 Xavier Leroy (Xavier.Leroy@inria.fr)              */
/*                                                                      */
/* This program is free software; you can redistribute it and/or        */
/* modify it under the terms of the GNU Library General Public License  */
/* as published by the Free Software Foundation; either version 2       */
/* of the License, or (at your option) any later version.               */
/*                                                                      */
/* This program is distributed in the hope that it will be useful,      */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of       */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        */
/* GNU Library General Public License for more details.                 */

/* Mutexes */

#include <errno.h>
#include <sched.h>
#include <stddef.h>
#include "pthread.h"
#include "internals.h"
#include "spinlock.h"
#include "queue.h"
#include "restart.h"

int __pthread_mutex_init(pthread_mutex_t * mutex,
                       const pthread_mutexattr_t * mutex_attr)
{
  mutex->m_spinlock = 0;
  mutex->m_count = 0;
  mutex->m_owner = NULL;
  mutex->m_kind =
    mutex_attr == NULL ? PTHREAD_MUTEX_FAST_NP : mutex_attr->mutexkind;
  queue_init(&mutex->m_waiting);
  return 0;
}
weak_alias (__pthread_mutex_init, pthread_mutex_init)

int __pthread_mutex_destroy(pthread_mutex_t * mutex)
{
  int count;
  acquire(&mutex->m_spinlock);
  count = mutex->m_count;
  release(&mutex->m_spinlock);
  if (count > 0) return EBUSY;
  return 0;
}
weak_alias (__pthread_mutex_destroy, pthread_mutex_destroy)

int __pthread_mutex_trylock(pthread_mutex_t * mutex)
{
  pthread_descr self;

  acquire(&mutex->m_spinlock);
  switch(mutex->m_kind) {
  case PTHREAD_MUTEX_FAST_NP:
    if (mutex->m_count == 0) {
      mutex->m_count = 1;
      release(&mutex->m_spinlock);
      return 0;
    }
    break;
  case PTHREAD_MUTEX_RECURSIVE_NP:
    self = thread_self();
    if (mutex->m_count == 0 || mutex->m_owner == self) {
      mutex->m_count++;
      mutex->m_owner = self;
      release(&mutex->m_spinlock);
      return 0;
    }
    break;
  case PTHREAD_MUTEX_ERRORCHECK_NP:
    self = thread_self();
    if (mutex->m_count == 0) {
      mutex->m_count = 1;
      mutex->m_owner = self;
      release(&mutex->m_spinlock);
      return 0;
    }
    break;
  default:
    release(&mutex->m_spinlock);
    return EINVAL;
  }
  release(&mutex->m_spinlock);
  return EBUSY;
}
weak_alias (__pthread_mutex_trylock, pthread_mutex_trylock)

int __pthread_mutex_lock(pthread_mutex_t * mutex)
{
  pthread_descr self;

  acquire(&mutex->m_spinlock);
  switch(mutex->m_kind) {
  case PTHREAD_MUTEX_FAST_NP:
    if (mutex->m_count == 0) {
      mutex->m_count = 1;
      release(&mutex->m_spinlock);
      return 0;
    }
    self = thread_self();
    break;
  case PTHREAD_MUTEX_RECURSIVE_NP:
    self = thread_self();
    if (mutex->m_count == 0 || mutex->m_owner == self) {
      mutex->m_count++;
      mutex->m_owner = self;
      release(&mutex->m_spinlock);
      return 0;
    }
    break;
  case PTHREAD_MUTEX_ERRORCHECK_NP:
    self = thread_self();
    if (mutex->m_count == 0) {
      mutex->m_count = 1;
      mutex->m_owner = self;
      release(&mutex->m_spinlock);
      return 0;
    } else if (mutex->m_owner == self) {
      release(&mutex->m_spinlock);
      return EDEADLK;
    }
    break;
  default:
    release(&mutex->m_spinlock);
    return EINVAL;
  }
  /* Suspend ourselves */
  enqueue(&mutex->m_waiting, self);
  release(&mutex->m_spinlock);
  suspend(self); /* This is not a cancellation point */
  /* Now we own the mutex */
  ASSERT(mutex->m_count == 1);
  mutex->m_owner = self;        /* for recursive and errorcheck mutexes */
  return 0;
}
weak_alias (__pthread_mutex_lock, pthread_mutex_lock)

int __pthread_mutex_unlock(pthread_mutex_t * mutex)
{
  pthread_descr th;

  acquire(&mutex->m_spinlock);
  switch (mutex->m_kind) {
  case PTHREAD_MUTEX_FAST_NP:
    break;
  case PTHREAD_MUTEX_RECURSIVE_NP:
    if (mutex->m_count >= 2) {
      mutex->m_count--;
      release(&mutex->m_spinlock);
      return 0;
    }
    break;
  case PTHREAD_MUTEX_ERRORCHECK_NP:
    if (mutex->m_count == 0 || mutex->m_owner != thread_self()) {
      release(&mutex->m_spinlock);
      return EPERM;
    }
    break;
  default:
    release(&mutex->m_spinlock);
    return EINVAL;
  }
  th = dequeue(&mutex->m_waiting);
  /* If no waiters, unlock the mutex */
  if (th == NULL) mutex->m_count = 0;
  release(&mutex->m_spinlock);
  /* If there is a waiter, restart it with the mutex still locked */
  if (th != NULL) {
    mutex->m_owner = NULL;      /* we no longer own the mutex */
    restart(th);
  }
  return 0;
}
weak_alias (__pthread_mutex_unlock, pthread_mutex_unlock)

int __pthread_mutexattr_init(pthread_mutexattr_t *attr)
{
  attr->mutexkind = PTHREAD_MUTEX_FAST_NP;
  return 0;
}
weak_alias (__pthread_mutexattr_init, pthread_mutexattr_init)

int __pthread_mutexattr_destroy(pthread_mutexattr_t *attr)
{
  return 0;
}
weak_alias (__pthread_mutexattr_destroy, pthread_mutexattr_destroy)

int __pthread_mutexattr_setkind_np(pthread_mutexattr_t *attr, int kind)
{
  if (kind != PTHREAD_MUTEX_FAST_NP
      && kind != PTHREAD_MUTEX_RECURSIVE_NP
      && kind != PTHREAD_MUTEX_ERRORCHECK_NP)
    return EINVAL;
  attr->mutexkind = kind;
  return 0;
}
weak_alias (__pthread_mutexattr_setkind_np, pthread_mutexattr_setkind_np)

int __pthread_mutexattr_getkind_np(const pthread_mutexattr_t *attr, int *kind)
{
  *kind = attr->mutexkind;
  return 0;
}
weak_alias (__pthread_mutexattr_getkind_np, pthread_mutexattr_getkind_np)

/* Once-only execution */

static pthread_mutex_t once_masterlock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t once_finished = PTHREAD_COND_INITIALIZER;

enum { NEVER = 0, IN_PROGRESS = 1, DONE = 2 };

int __pthread_once(pthread_once_t * once_control, void (*init_routine)(void))
{
  /* Test without locking first for speed */
  if (*once_control == DONE) return 0;
  /* Lock and test again */
  pthread_mutex_lock(&once_masterlock);
  /* If init_routine is being called from another routine, wait until
     it completes. */
  while (*once_control == IN_PROGRESS) {
    pthread_cond_wait(&once_finished, &once_masterlock);
  }
  /* Here *once_control is stable and either NEVER or DONE. */
  if (*once_control == NEVER) {
    *once_control = IN_PROGRESS;
    pthread_mutex_unlock(&once_masterlock);
    init_routine();
    pthread_mutex_lock(&once_masterlock);
    *once_control = DONE;
    pthread_cond_broadcast(&once_finished);
  }
  pthread_mutex_unlock(&once_masterlock);
  return 0;
}
weak_alias (__pthread_once, pthread_once)
