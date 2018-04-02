/* pthread_hurd_cond_timedwait_np.  Hurd-specific wait on a condition.
   Copyright (C) 2012-2018 Free Software Foundation, Inc.
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
#include <assert.h>
#include <hurd/signal.h>

#include <pt-internal.h>

extern int __pthread_hurd_cond_timedwait_internal (pthread_cond_t *cond,
						   pthread_mutex_t *mutex,
						   const struct timespec
						   *abstime);

int
__pthread_hurd_cond_timedwait_np (pthread_cond_t *cond,
				  pthread_mutex_t *mutex,
				  const struct timespec *abstime)
{
  return __pthread_hurd_cond_timedwait_internal (cond, mutex, abstime);
}

strong_alias (__pthread_hurd_cond_timedwait_np, pthread_hurd_cond_timedwait_np);

int
__pthread_hurd_cond_timedwait_internal (pthread_cond_t *cond,
					pthread_mutex_t *mutex,
					const struct timespec *abstime)
{
  struct hurd_sigstate *ss = _hurd_self_sigstate ();
  struct __pthread *self = _pthread_self ();
  error_t err = 0;
  int cancel, drain;
  clockid_t clock_id = __pthread_default_condattr.__clock;

  /* This function will be called by hurd_thread_cancel while we are blocked
     We wake up our thread if it's still blocking or about to block, so it will
     progress and notice the cancellation flag.  */
  void cancel_me (void)
  {
    int unblock;

    __pthread_spin_lock (&cond->__lock);
    /* The thread only needs to be awaken if it's blocking or about to block.
       If it was already unblocked, it's not queued any more.  */
    unblock = self->prevp != NULL;
    if (unblock)
      __pthread_dequeue (self);
    __pthread_spin_unlock (&cond->__lock);

    if (unblock)
      __pthread_wakeup (self);
  }

  assert (ss->intr_port == MACH_PORT_NULL);	/* Sanity check for signal bugs. */

  if (abstime != NULL && (abstime->tv_nsec < 0 || abstime->tv_nsec >= 1000000000))
    return EINVAL;

  /* Atomically enqueue our thread on the condition variable's queue of
     waiters, and mark our sigstate to indicate that `cancel_me' must be
     called to wake us up.  We must hold the sigstate lock while acquiring
     the condition variable's lock and tweaking it, so that
     hurd_thread_cancel can never suspend us and then deadlock waiting for
     the condition variable's lock.  */

  __spin_lock (&ss->lock);
  __pthread_spin_lock (&cond->__lock);
  cancel = ss->cancel;
  if (cancel)
    /* We were cancelled before doing anything.  Don't block at all.  */
    ss->cancel = 0;
  else
    {
      /* Put us on the queue so that pthread_cond_broadcast will know to wake
         us up.  */
      __pthread_enqueue (&cond->__queue, self);
      if (cond->__attr)
	clock_id = cond->__attr->__clock;
      /* Tell hurd_thread_cancel how to unblock us.  */
      ss->cancel_hook = &cancel_me;
    }
  __pthread_spin_unlock (&cond->__lock);
  __spin_unlock (&ss->lock);

  if (cancel)
    {
      /* Cancelled on entry.  Just leave the mutex locked.  */
      mutex = NULL;

      __spin_lock (&ss->lock);
    }
  else
    {
      /* Release MUTEX before blocking.  */
      __pthread_mutex_unlock (mutex);

      /* Block the thread.  */
      if (abstime != NULL)
	err = __pthread_timedblock (self, abstime, clock_id);
      else
	{
	  err = 0;
	  __pthread_block (self);
	}

      /* As it was done when enqueueing, prevent hurd_thread_cancel from
         suspending us while the condition lock is held.  */
      __spin_lock (&ss->lock);
      __pthread_spin_lock (&cond->__lock);
      if (self->prevp == NULL)
	/* Another thread removed us from the list of waiters, which means
	   a wakeup message has been sent.  It was either consumed while
	   we were blocking, or queued after we timed out and before we
	   acquired the condition lock, in which case the message queue
	   must be drained.  */
	drain = err ? 1 : 0;
      else
	{
	  /* We're still in the list of waiters.  Noone attempted to wake us
	     up, i.e. we timed out.  */
	  __pthread_dequeue (self);
	  drain = 0;
	}
      __pthread_spin_unlock (&cond->__lock);

      if (drain)
	__pthread_block (self);
    }

  /* Clear the hook, now that we are done blocking.  */
  ss->cancel_hook = NULL;
  /* Check the cancellation flag; we might have unblocked due to
     cancellation rather than a normal pthread_cond_signal or
     pthread_cond_broadcast (or we might have just happened to get cancelled
     right after waking up).  */
  cancel |= ss->cancel;
  ss->cancel = 0;
  __spin_unlock (&ss->lock);

  if (mutex != NULL)
    /* Reacquire the mutex and return.  */
    __pthread_mutex_lock (mutex);

  if (cancel)
    return EINTR;
  else if (err)
    {
      assert (err == ETIMEDOUT);
      return err;
    }

  return 0;
}
