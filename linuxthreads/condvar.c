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

static int pthread_cond_timedwait_relative_old(pthread_cond_t *,
    pthread_mutex_t *, struct timespec *);

static int pthread_cond_timedwait_relative_new(pthread_cond_t *,
    pthread_mutex_t *, struct timespec *);

static int (*pthread_cond_tw_rel)(pthread_cond_t *, pthread_mutex_t *,
    struct timespec *) = pthread_cond_timedwait_relative_old;

/* initialize this module */
void __pthread_init_condvar(int rt_sig_available)
{
  if (rt_sig_available)
    pthread_cond_tw_rel = pthread_cond_timedwait_relative_new;
}

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

/* Function called by pthread_cancel to remove the thread from
   waiting on a condition variable queue. */

static int cond_extricate_func(void *obj, pthread_descr th)
{
  volatile pthread_descr self = thread_self();
  pthread_cond_t *cond = obj;
  int did_remove = 0;

  __pthread_lock(&cond->__c_lock, self);
  did_remove = remove_from_queue(&cond->__c_waiting, th);
  __pthread_unlock(&cond->__c_lock);

  return did_remove;
}

int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{
  volatile pthread_descr self = thread_self();
  pthread_extricate_if extr;
  int already_canceled = 0;

  /* Set up extrication interface */
  extr.pu_object = cond;
  extr.pu_extricate_func = cond_extricate_func;

  /* Register extrication interface */
  __pthread_set_own_extricate_if(self, &extr);

  /* Atomically enqueue thread for waiting, but only if it is not
     canceled. If the thread is canceled, then it will fall through the
     suspend call below, and then call pthread_exit without
     having to worry about whether it is still on the condition variable queue.
     This depends on pthread_cancel setting p_canceled before calling the
     extricate function. */

  __pthread_lock(&cond->__c_lock, self);
  if (!(THREAD_GETMEM(self, p_canceled)
      && THREAD_GETMEM(self, p_cancelstate) == PTHREAD_CANCEL_ENABLE))
    enqueue(&cond->__c_waiting, self);
  else
    already_canceled = 1;
  __pthread_unlock(&cond->__c_lock);

  if (already_canceled) {
    __pthread_set_own_extricate_if(self, 0);
    pthread_exit(PTHREAD_CANCELED);
  }

  pthread_mutex_unlock(mutex);

  suspend(self);
  __pthread_set_own_extricate_if(self, 0);

  /* Check for cancellation again, to provide correct cancellation
     point behavior */

  if (THREAD_GETMEM(self, p_woken_by_cancel)
      && THREAD_GETMEM(self, p_cancelstate) == PTHREAD_CANCEL_ENABLE) {
    THREAD_SETMEM(self, p_woken_by_cancel, 0);
    pthread_mutex_lock(mutex);
    pthread_exit(PTHREAD_CANCELED);
  }

  pthread_mutex_lock(mutex);
  return 0;
}

/* The following function is used on kernels that don't have rt signals.
   SIGUSR1 is used as the restart signal. The different code is needed
   because that ordinary signal does not queue. */

static int
pthread_cond_timedwait_relative_old(pthread_cond_t *cond,
				pthread_mutex_t *mutex,
				struct timespec * reltime)
{
  volatile pthread_descr self = thread_self();
  sigset_t unblock, initial_mask;
  int retsleep, already_canceled, was_signalled;
  sigjmp_buf jmpbuf;
  pthread_extricate_if extr;

requeue_and_wait_again:

  retsleep = 0;
  already_canceled = 0;
  was_signalled = 0;

  /* Set up extrication interface */
  extr.pu_object = cond;
  extr.pu_extricate_func = cond_extricate_func;

  /* Register extrication interface */
  __pthread_set_own_extricate_if(self, &extr);

  /* Enqueue to wait on the condition and check for cancellation. */
  __pthread_lock(&cond->__c_lock, self);
  if (!(THREAD_GETMEM(self, p_canceled)
      && THREAD_GETMEM(self, p_cancelstate) == PTHREAD_CANCEL_ENABLE))
    enqueue(&cond->__c_waiting, self);
  else
    already_canceled = 1;
  __pthread_unlock(&cond->__c_lock);

  if (already_canceled) {
    __pthread_set_own_extricate_if(self, 0);
    pthread_exit(PTHREAD_CANCELED);
  }

  pthread_mutex_unlock(mutex);

  if (atomic_decrement(&self->p_resume_count) == 0) {
    /* Set up a longjmp handler for the restart signal, unblock
       the signal and sleep. */

    if (sigsetjmp(jmpbuf, 1) == 0) {
      THREAD_SETMEM(self, p_signal_jmp, &jmpbuf);
      THREAD_SETMEM(self, p_signal, 0);
      /* Unblock the restart signal */
      sigemptyset(&unblock);
      sigaddset(&unblock, __pthread_sig_restart);
      sigprocmask(SIG_UNBLOCK, &unblock, &initial_mask);
      /* Sleep for the required duration */
      retsleep = __libc_nanosleep(reltime, reltime);
      /* Block the restart signal again */
      sigprocmask(SIG_SETMASK, &initial_mask, NULL);
      was_signalled = 0;
    } else {
      retsleep = -1;
      was_signalled = 1;
    }
    THREAD_SETMEM(self, p_signal_jmp, NULL);
  }

  /* Now was_signalled is true if we exited the above code
     due to the delivery of a restart signal.  In that case,
     we know we have been dequeued and resumed and that the
     resume count is balanced.  Otherwise, there are some
     cases to consider. First, try to bump up the resume count
     back to zero. If it goes to 1, it means restart() was
     invoked on this thread. The signal must be consumed
     and the count bumped down and everything is cool.
     Otherwise, no restart was delivered yet, so we remove
     the thread from the queue. If this succeeds, it's a clear
     case of timeout. If we fail to remove from the queue, then we
     must wait for a restart. */

  if (!was_signalled) {
    if (atomic_increment(&self->p_resume_count) != -1) {
      __pthread_wait_for_restart_signal(self);
      atomic_decrement(&self->p_resume_count); /* should be zero now! */
    } else {
      int was_on_queue;
      __pthread_lock(&cond->__c_lock, self);
      was_on_queue = remove_from_queue(&cond->__c_waiting, self);
      __pthread_unlock(&cond->__c_lock);

      if (was_on_queue) {
	__pthread_set_own_extricate_if(self, 0);
	pthread_mutex_lock(mutex);

	if (retsleep == 0)
	  return ETIMEDOUT;
	/* Woken by a signal: resume waiting as required by Single Unix
	   Specification.  */
	goto requeue_and_wait_again;
      }

      suspend(self);
    }
  }

  __pthread_set_own_extricate_if(self, 0);

  /* The remaining logic is the same as in other cancellable waits,
     such as pthread_join sem_wait or pthread_cond wait. */

  if (THREAD_GETMEM(self, p_woken_by_cancel)
      && THREAD_GETMEM(self, p_cancelstate) == PTHREAD_CANCEL_ENABLE) {
    THREAD_SETMEM(self, p_woken_by_cancel, 0);
    pthread_mutex_lock(mutex);
    pthread_exit(PTHREAD_CANCELED);
  }

  pthread_mutex_lock(mutex);
  return 0;
}

/* The following function is used on new (late 2.1 and 2.2 and higher) kernels
   that have rt signals which queue. */

static int
pthread_cond_timedwait_relative_new(pthread_cond_t *cond,
				pthread_mutex_t *mutex,
				struct timespec * reltime)
{
  volatile pthread_descr self = thread_self();
  sigset_t unblock, initial_mask;
  int retsleep, already_canceled, was_signalled;
  sigjmp_buf jmpbuf;
  pthread_extricate_if extr;

 requeue_and_wait_again:

  retsleep = 0;
  already_canceled = 0;
  was_signalled = 0;

  /* Set up extrication interface */
  extr.pu_object = cond;
  extr.pu_extricate_func = cond_extricate_func;

  /* Register extrication interface */
  __pthread_set_own_extricate_if(self, &extr);

  /* Enqueue to wait on the condition and check for cancellation. */
  __pthread_lock(&cond->__c_lock, self);
  if (!(THREAD_GETMEM(self, p_canceled)
      && THREAD_GETMEM(self, p_cancelstate) == PTHREAD_CANCEL_ENABLE))
    enqueue(&cond->__c_waiting, self);
  else
    already_canceled = 1;
  __pthread_unlock(&cond->__c_lock);

  if (already_canceled) {
    __pthread_set_own_extricate_if(self, 0);
    pthread_exit(PTHREAD_CANCELED);
  }

  pthread_mutex_unlock(mutex);

  /* Set up a longjmp handler for the restart signal, unblock
     the signal and sleep. */

  if (sigsetjmp(jmpbuf, 1) == 0) {
    THREAD_SETMEM(self, p_signal_jmp, &jmpbuf);
    THREAD_SETMEM(self, p_signal, 0);
    /* Unblock the restart signal */
    sigemptyset(&unblock);
    sigaddset(&unblock, __pthread_sig_restart);
    sigprocmask(SIG_UNBLOCK, &unblock, &initial_mask);
    /* Sleep for the required duration */
    retsleep = __libc_nanosleep(reltime, reltime);
    /* Block the restart signal again */
    sigprocmask(SIG_SETMASK, &initial_mask, NULL);
    was_signalled = 0;
  } else {
    retsleep = -1;
    was_signalled = 1;
  }
  THREAD_SETMEM(self, p_signal_jmp, NULL);

  /* Now was_signalled is true if we exited the above code
     due to the delivery of a restart signal.  In that case,
     everything is cool. We have been removed from the queue
     by the other thread, and consumed its signal.

     Otherwise we this thread woke up spontaneously, or due to a signal other
     than restart. The next thing to do is to try to remove the thread
     from the queue. This may fail due to a race against another thread
     trying to do the same. In the failed case, we know we were signalled,
     and we may also have to consume a restart signal. */

  if (!was_signalled) {
    int was_on_queue;

    /* __pthread_lock will queue back any spurious restarts that
       may happen to it. */

    __pthread_lock(&cond->__c_lock, self);
    was_on_queue = remove_from_queue(&cond->__c_waiting, self);
    __pthread_unlock(&cond->__c_lock);

    if (was_on_queue) {
      __pthread_set_own_extricate_if(self, 0);
      pthread_mutex_lock(mutex);

      if (retsleep == 0)
	return ETIMEDOUT;
      /* Woken by a signal: resume waiting as required by Single Unix
	 Specification.  */
      goto requeue_and_wait_again;
    }

    /* Eat the outstanding restart() from the signaller */
    suspend(self);
  }

  __pthread_set_own_extricate_if(self, 0);

  /* The remaining logic is the same as in other cancellable waits,
     such as pthread_join sem_wait or pthread_cond wait. */

  if (THREAD_GETMEM(self, p_woken_by_cancel)
      && THREAD_GETMEM(self, p_cancelstate) == PTHREAD_CANCEL_ENABLE) {
    THREAD_SETMEM(self, p_woken_by_cancel, 0);
    pthread_mutex_lock(mutex);
    pthread_exit(PTHREAD_CANCELED);
  }

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

  /* Indirect call through pointer! */
  return pthread_cond_tw_rel(cond, mutex, &reltime);
}

int pthread_cond_signal(pthread_cond_t *cond)
{
  pthread_descr th;

  __pthread_lock(&cond->__c_lock, NULL);
  th = dequeue(&cond->__c_waiting);
  __pthread_unlock(&cond->__c_lock);
  if (th != NULL) restart(th);
  return 0;
}

int pthread_cond_broadcast(pthread_cond_t *cond)
{
  pthread_descr tosignal, th;

  __pthread_lock(&cond->__c_lock, NULL);
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
