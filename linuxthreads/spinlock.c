/* Linuxthreads - a simple clone()-based implementation of Posix        */
/* threads for Linux.                                                   */
/* Copyright (C) 1998 Xavier Leroy (Xavier.Leroy@inria.fr)              */
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

/* Internal locks */

#include <errno.h>
#include <sched.h>
#include <time.h>
#include "pthread.h"
#include "internals.h"
#include "spinlock.h"
#include "restart.h"

/* The status field of a fastlock has the following meaning:
     0: fastlock is free
     1: fastlock is taken, no thread is waiting on it
  ADDR: fastlock is taken, ADDR is address of thread descriptor for
        first waiting thread, other waiting threads are linked via
        their p_nextwaiting field.
   The waiting list is not sorted by priority order.
   Actually, we always insert at top of list (sole insertion mode
   that can be performed without locking).
   For __pthread_unlock, we perform a linear search in the list
   to find the highest-priority, oldest waiting thread.
   This is safe because there are no concurrent __pthread_unlock
   operations -- only the thread that locked the mutex can unlock it. */

void __pthread_lock(struct _pthread_fastlock * lock)
{
  long oldstatus, newstatus;
  pthread_descr self = NULL;

  do {
    oldstatus = lock->status;
    if (oldstatus == 0) {
      newstatus = 1;
    } else {
      self = thread_self();
      self->p_nextwaiting = (pthread_descr) oldstatus;
      newstatus = (long) self;
    }
  } while(! compare_and_swap(&lock->status, oldstatus, newstatus,
                             &lock->spinlock));
  if (oldstatus != 0) suspend(self);
}

int __pthread_trylock(struct _pthread_fastlock * lock)
{
  long oldstatus;

  do {
    oldstatus = lock->status;
    if (oldstatus != 0) return EBUSY;
  } while(! compare_and_swap(&lock->status, 0, 1, &lock->spinlock));
  return 0;
}

void __pthread_unlock(struct _pthread_fastlock * lock)
{
  long oldstatus;
  pthread_descr thr, * ptr, * maxptr;
  int maxprio;

again:
  oldstatus = lock->status;
  if (oldstatus == 1) {
    /* No threads are waiting for this lock */
    if (! compare_and_swap(&lock->status, 1, 0, &lock->spinlock)) goto again;
    return;
  }
  /* Find thread in waiting queue with maximal priority */
  ptr = (pthread_descr *) &lock->status;
  thr = (pthread_descr) oldstatus;
  maxprio = 0;
  maxptr = ptr;
  while (thr != (pthread_descr) 1) {
    if (thr->p_priority >= maxprio) {
      maxptr = ptr;
      maxprio = thr->p_priority;
    }
    ptr = &(thr->p_nextwaiting);
    thr = *ptr;
  }
  /* Remove max prio thread from waiting list. */
  if (maxptr == (pthread_descr *) &lock->status) {
    /* If max prio thread is at head, remove it with compare-and-swap
       to guard against concurrent lock operation */
    thr = (pthread_descr) oldstatus;
    if (! compare_and_swap(&lock->status,
                           oldstatus, (long)(thr->p_nextwaiting),
                           &lock->spinlock))
      goto again;
  } else {
    /* No risk of concurrent access, remove max prio thread normally */
    thr = *maxptr;
    *maxptr = thr->p_nextwaiting;
  }
  /* Wake up the selected waiting thread */
  thr->p_nextwaiting = NULL;
  restart(thr);
}

/* Compare-and-swap emulation with a spinlock */

#ifdef TEST_FOR_COMPARE_AND_SWAP
int __pthread_has_cas = 0;
#endif

#if !defined HAS_COMPARE_AND_SWAP || defined TEST_FOR_COMPARE_AND_SWAP

static void __pthread_acquire(int * spinlock);

int __pthread_compare_and_swap(long * ptr, long oldval, long newval,
                               int * spinlock)
{
  int res;
  if (testandset(spinlock)) __pthread_acquire(spinlock);
  if (*ptr == oldval) {
    *ptr = newval; res = 1;
  } else {
    res = 0;
  }
  *spinlock = 0;
  return res;
}

/* This function is called if the inlined test-and-set
   in __pthread_compare_and_swap() failed */

/* The retry strategy is as follows:
   - We test and set the spinlock MAX_SPIN_COUNT times, calling
     sched_yield() each time.  This gives ample opportunity for other
     threads with priority >= our priority to make progress and
     release the spinlock.
   - If a thread with priority < our priority owns the spinlock,
     calling sched_yield() repeatedly is useless, since we're preventing
     the owning thread from making progress and releasing the spinlock.
     So, after MAX_SPIN_LOCK attemps, we suspend the calling thread
     using nanosleep().  This again should give time to the owning thread
     for releasing the spinlock.
     Notice that the nanosleep() interval must not be too small,
     since the kernel does busy-waiting for short intervals in a realtime
     process (!).  The smallest duration that guarantees thread
     suspension is currently 2ms.
   - When nanosleep() returns, we try again, doing MAX_SPIN_COUNT
     sched_yield(), then sleeping again if needed. */

static void __pthread_acquire(int * spinlock)
{
  int cnt = 0;
  struct timespec tm;

  while (testandset(spinlock)) {
    if (cnt < MAX_SPIN_COUNT) {
      sched_yield();
      cnt++;
    } else {
      tm.tv_sec = 0;
      tm.tv_nsec = SPIN_SLEEP_DURATION;
      nanosleep(&tm, NULL);
      cnt = 0;
    }
  }
}

#endif
