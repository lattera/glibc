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

/* Semaphores a la POSIX 1003.1b */

#include <errno.h>
#include "pthread.h"
#include "semaphore.h"
#include "internals.h"
#include "spinlock.h"
#include "restart.h"
#include "queue.h"

int sem_init(sem_t *sem, int pshared, unsigned int value)
{
  if (value > SEM_VALUE_MAX) {
    errno = EINVAL;
    return -1;
  }
  if (pshared) {
    errno = ENOSYS;
    return -1;
  }
  __pthread_init_lock((struct _pthread_fastlock *) &sem->sem_lock);
  sem->sem_value = value;
  sem->sem_waiting = NULL;
  return 0;
}

int sem_wait(sem_t * sem)
{
  volatile pthread_descr self;

  __pthread_lock((struct _pthread_fastlock *) &sem->sem_lock);
  if (sem->sem_value > 0) {
    sem->sem_value--;
    __pthread_unlock((struct _pthread_fastlock *) &sem->sem_lock);
    return 0;
  }
  self = thread_self();
  enqueue(&sem->sem_waiting, self);
  /* Wait for sem_post or cancellation */
  __pthread_unlock((struct _pthread_fastlock *) &sem->sem_lock);
  suspend_with_cancellation(self);
  /* This is a cancellation point */
  if (self->p_canceled && self->p_cancelstate == PTHREAD_CANCEL_ENABLE) {
    /* Remove ourselves from the waiting list if we're still on it */
    __pthread_lock((struct _pthread_fastlock *) &sem->sem_lock);
    remove_from_queue(&sem->sem_waiting, self);
    __pthread_unlock((struct _pthread_fastlock *) &sem->sem_lock);
    pthread_exit(PTHREAD_CANCELED);
  }
  /* We got the semaphore */
  return 0;
}

int sem_trywait(sem_t * sem)
{
  int retval;

  __pthread_lock((struct _pthread_fastlock *) &sem->sem_lock);
  if (sem->sem_value == 0) {
    errno = EAGAIN;
    retval = -1;
  } else {
    sem->sem_value--;
    retval = 0;
  }
  return retval;
}

int sem_post(sem_t * sem)
{
  pthread_descr self = thread_self();
  pthread_descr th;
  struct pthread_request request;

  if (self->p_in_sighandler == NULL) {
    __pthread_lock((struct _pthread_fastlock *) &sem->sem_lock);
    if (sem->sem_waiting == NULL) {
      if (sem->sem_value >= SEM_VALUE_MAX) {
        /* Overflow */
        errno = ERANGE;
        __pthread_unlock((struct _pthread_fastlock *) &sem->sem_lock);
        return -1;
      }
      sem->sem_value++;
      __pthread_unlock((struct _pthread_fastlock *) &sem->sem_lock);
    } else {
      th = dequeue(&sem->sem_waiting);
      __pthread_unlock((struct _pthread_fastlock *) &sem->sem_lock);
      restart(th);
    }
  } else {
    /* If we're in signal handler, delegate post operation to
       the thread manager. */
    if (__pthread_manager_request < 0) {
      if (__pthread_initialize_manager() < 0) {
        errno = EAGAIN;
        return -1;
      }
    }
    request.req_kind = REQ_POST;
    request.req_args.post = sem;
    __libc_write(__pthread_manager_request,
                 (char *) &request, sizeof(request));
  }
  return 0;
}

int sem_getvalue(sem_t * sem, int * sval)
{
  *sval = sem->sem_value;
  return 0;
}

int sem_destroy(sem_t * sem)
{
  if (sem->sem_waiting != NULL) {
    errno = EBUSY;
    return -1;
  }
  return 0;
}
