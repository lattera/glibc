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

static void remove_from_queue(pthread_queue * q, pthread_descr th);

int pthread_cond_init(pthread_cond_t *cond,
                      const pthread_condattr_t *cond_attr)
{
  cond->c_spinlock = 0;
  queue_init(&cond->c_waiting);
  return 0;
}

int pthread_cond_destroy(pthread_cond_t *cond)
{
  pthread_descr head;

  acquire(&cond->c_spinlock);
  head = cond->c_waiting.head;
  release(&cond->c_spinlock);
  if (head != NULL) return EBUSY;
  return 0;
}

int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{
  volatile pthread_descr self = thread_self();
  acquire(&cond->c_spinlock);
  enqueue(&cond->c_waiting, self);
  release(&cond->c_spinlock);
  pthread_mutex_unlock(mutex);
  suspend_with_cancellation(self);
  pthread_mutex_lock(mutex);
  /* This is a cancellation point */
  if (self->p_canceled && self->p_cancelstate == PTHREAD_CANCEL_ENABLE) {
    /* Remove ourselves from the waiting queue if we're still on it */
    acquire(&cond->c_spinlock);
    remove_from_queue(&cond->c_waiting, self);
    release(&cond->c_spinlock);
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
  acquire(&cond->c_spinlock);
  enqueue(&cond->c_waiting, self);
  release(&cond->c_spinlock);
  pthread_mutex_unlock(mutex);
  /* Set up a longjmp handler for the restart signal */
  /* No need to save the signal mask, since PTHREAD_SIG_RESTART will be
     blocked when doing the siglongjmp, and we'll just leave it blocked. */
  if (sigsetjmp(jmpbuf, 0) == 0) {
    self->p_signal_jmp = &jmpbuf;
    self->p_signal = 0;
    /* Check for cancellation */
    if (self->p_canceled && self->p_cancelstate == PTHREAD_CANCEL_ENABLE) {
      retsleep = -1;
    } else {
      /* Unblock the restart signal */
      sigemptyset(&unblock);
      sigaddset(&unblock, PTHREAD_SIG_RESTART);
      sigprocmask(SIG_UNBLOCK, &unblock, &initial_mask);
      /* Sleep for the required duration */
      retsleep = __libc_nanosleep(reltime, NULL);
      /* Block the restart signal again */
      sigprocmask(SIG_SETMASK, &initial_mask, NULL);
    }
  } else {
    retsleep = -1;
  }
  self->p_signal_jmp = NULL;
  /* Here, either the condition was signaled (self->p_signal != 0)
                   or we got canceled (self->p_canceled != 0)
                   or the timeout occurred (retsleep == 0)
                   or another interrupt occurred (retsleep == -1) */
  /* Re-acquire the spinlock */
  acquire(&cond->c_spinlock);
  /* This is a cancellation point */
  if (self->p_canceled && self->p_cancelstate == PTHREAD_CANCEL_ENABLE) {
    remove_from_queue(&cond->c_waiting, self);
    release(&cond->c_spinlock);
    pthread_mutex_lock(mutex);
    pthread_exit(PTHREAD_CANCELED);
  }
  /* If not signaled: also remove ourselves and return an error code */
  if (self->p_signal == 0) {
    remove_from_queue(&cond->c_waiting, self);
    release(&cond->c_spinlock);
    pthread_mutex_lock(mutex);
    return retsleep == 0 ? ETIMEDOUT : EINTR;
  }
  /* Otherwise, return normally */
  release(&cond->c_spinlock);
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

  acquire(&cond->c_spinlock);
  th = dequeue(&cond->c_waiting);
  release(&cond->c_spinlock);
  if (th != NULL) restart(th);
  return 0;
}

int pthread_cond_broadcast(pthread_cond_t *cond)
{
  pthread_queue tosignal;
  pthread_descr th;

  acquire(&cond->c_spinlock);
  /* Copy the current state of the waiting queue and empty it */
  tosignal = cond->c_waiting;
  queue_init(&cond->c_waiting);
  release(&cond->c_spinlock);
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

/* Auxiliary function on queues */

static void remove_from_queue(pthread_queue * q, pthread_descr th)
{
  pthread_descr t;

  if (q->head == NULL) return;
  if (q->head == th) {
    q->head = th->p_nextwaiting;
    if (q->head == NULL) q->tail = NULL;
    th->p_nextwaiting = NULL;
    return;
  }
  for (t = q->head; t->p_nextwaiting != NULL; t = t->p_nextwaiting) {
    if (t->p_nextwaiting == th) {
      t->p_nextwaiting = th->p_nextwaiting;
      if (th->p_nextwaiting == NULL) q->tail = t;
      th->p_nextwaiting = NULL;
      return;
    }
  }
}
