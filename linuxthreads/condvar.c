/* Linuxthreads - a simple clone()-based implementation of Posix        */
/* threads for Linux.                                                   */
/* Copyright (C) 1996 Xavier Leroy (Xavier.Leroy@inria.fr)              */
/* and Pavel Krauz (krauz@fsid.cvut.cz).                                */
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

/* Condition variables */

#include <errno.h>
#include <sched.h>
#include <stddef.h>
#include <sys/time.h>
#include "pthread.h"
#include "internals.h"
#include "spinlock.h"
#include "queue.h"
#include "restart.h"

int pthread_cond_init(pthread_cond_t *cond,
                      const pthread_condattr_t *cond_attr)
{
  __pthread_init_lock(&cond->__c_lock);
  cond->__c_waiting = NULL;
  return 0;
}

int pthread_cond_destroy(pthread_cond_t *cond)
{
  if (cond->__c_waiting != NULL) return EBUSY;
  return 0;
}

int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{
  volatile pthread_descr self = thread_self();

  __pthread_lock(&cond->__c_lock);
  enqueue(&cond->__c_waiting, self);
  __pthread_unlock(&cond->__c_lock);
  pthread_mutex_unlock(mutex);
  suspend_with_cancellation(self);
  pthread_mutex_lock(mutex);
  /* This is a cancellation point */
  if (THREAD_GETMEM(self, p_canceled)
      && THREAD_GETMEM(self, p_cancelstate) == PTHREAD_CANCEL_ENABLE) {
    /* Remove ourselves from the waiting queue if we're still on it */
    __pthread_lock(&cond->__c_lock);
    remove_from_queue(&cond->__c_waiting, self);
    __pthread_unlock(&cond->__c_lock);
    pthread_exit(PTHREAD_CANCELED);
  }
  return 0;
}

static inline int
pthread_cond_timedwait_relative(pthread_cond_t *cond,
				pthread_mutex_t *mutex,
				const struct timespec * reltime)
{
  volatile pthread_descr self = thread_self();
  sigset_t unblock, initial_mask;
  int retsleep;
  sigjmp_buf jmpbuf;

  /* Wait on the condition */
  __pthread_lock(&cond->__c_lock);
  enqueue(&cond->__c_waiting, self);
  __pthread_unlock(&cond->__c_lock);
  pthread_mutex_unlock(mutex);
  /* Set up a longjmp handler for the restart and cancel signals */
  if (sigsetjmp(jmpbuf, 1) == 0) {
    THREAD_SETMEM(self, p_signal_jmp, &jmpbuf);
    THREAD_SETMEM(self, p_cancel_jmp, &jmpbuf);
    THREAD_SETMEM(self, p_signal, 0);
    /* Check for cancellation */
    if (THREAD_GETMEM(self, p_canceled)
	&& THREAD_GETMEM(self, p_cancelstate) == PTHREAD_CANCEL_ENABLE) {
      retsleep = -1;
    } else {
      /* Unblock the restart signal */
      sigemptyset(&unblock);
      sigaddset(&unblock, __pthread_sig_restart);
      sigprocmask(SIG_UNBLOCK, &unblock, &initial_mask);
      /* Sleep for the required duration */
      retsleep = __libc_nanosleep(reltime, NULL);
      /* Block the restart signal again */
      sigprocmask(SIG_SETMASK, &initial_mask, NULL);
    }
  } else {
    retsleep = -1;
  }
  THREAD_SETMEM(self, p_signal_jmp, NULL);
  THREAD_SETMEM(self, p_cancel_jmp, NULL);
  /* Here, either the condition was signaled (self->p_signal != 0)
                   or we got canceled (self->p_canceled != 0)
                   or the timeout occurred (retsleep == 0)
                   or another interrupt occurred (retsleep == -1) */
  /* This is a cancellation point */
  if (THREAD_GETMEM(self, p_canceled)
      && THREAD_GETMEM(self, p_cancelstate) == PTHREAD_CANCEL_ENABLE) {
    __pthread_lock(&cond->__c_lock);
    remove_from_queue(&cond->__c_waiting, self);
    __pthread_unlock(&cond->__c_lock);
    pthread_mutex_lock(mutex);
    pthread_exit(PTHREAD_CANCELED);
  }
  /* If not signaled: also remove ourselves and return an error code */
  if (THREAD_GETMEM(self, p_signal) == 0) {
    __pthread_lock(&cond->__c_lock);
    remove_from_queue(&cond->__c_waiting, self);
    __pthread_unlock(&cond->__c_lock);
    pthread_mutex_lock(mutex);
    return retsleep == 0 ? ETIMEDOUT : EINTR;
  }
  /* Otherwise, return normally */
  pthread_mutex_lock(mutex);
  return 0;
}

int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex,
                           const struct timespec * abstime)
{
  struct timeval now;
  struct timespec reltime;
  /* Compute a time offset relative to now */
  __gettimeofday(&now, NULL);
  reltime.tv_sec = abstime->tv_sec - now.tv_sec;
  reltime.tv_nsec = abstime->tv_nsec - now.tv_usec * 1000;
  if (reltime.tv_nsec < 0) {
    reltime.tv_nsec += 1000000000;
    reltime.tv_sec -= 1;
  }
  if (reltime.tv_sec < 0) return ETIMEDOUT;
  return pthread_cond_timedwait_relative(cond, mutex, &reltime);
}

int pthread_cond_signal(pthread_cond_t *cond)
{
  pthread_descr th;

  __pthread_lock(&cond->__c_lock);
  th = dequeue(&cond->__c_waiting);
  __pthread_unlock(&cond->__c_lock);
  if (th != NULL) restart(th);
  return 0;
}

int pthread_cond_broadcast(pthread_cond_t *cond)
{
  pthread_descr tosignal, th;

  __pthread_lock(&cond->__c_lock);
  /* Copy the current state of the waiting queue and empty it */
  tosignal = cond->__c_waiting;
  cond->__c_waiting = NULL;
  __pthread_unlock(&cond->__c_lock);
  /* Now signal each process in the queue */
  while ((th = dequeue(&tosignal)) != NULL) restart(th);
  return 0;
}

int pthread_condattr_init(pthread_condattr_t *attr)
{
  return 0;
}

int pthread_condattr_destroy(pthread_condattr_t *attr)
{
  return 0;
}
