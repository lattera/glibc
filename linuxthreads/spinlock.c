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
#include <stdlib.h>
#include <limits.h>
#include "pthread.h"
#include "internals.h"
#include "spinlock.h"
#include "restart.h"

#if !defined HAS_COMPARE_AND_SWAP || defined TEST_FOR_COMPARE_AND_SWAP
static void __pthread_acquire(int * spinlock);
#endif


/* The status field of a spinlock is a pointer whose least significant
   bit is a locked flag.

   Thus the field values have the following meanings:

   status == 0:       spinlock is free
   status == 1:       spinlock is taken; no thread is waiting on it

   (status & 1) == 1: spinlock is taken and (status & ~1L) is a
                      pointer to the first waiting thread; other
		      waiting threads are linked via the p_nextlock
		      field.
   (status & 1) == 0: same as above, but spinlock is not taken.

   The waiting list is not sorted by priority order.
   Actually, we always insert at top of list (sole insertion mode
   that can be performed without locking).
   For __pthread_unlock, we perform a linear search in the list
   to find the highest-priority, oldest waiting thread.
   This is safe because there are no concurrent __pthread_unlock
   operations -- only the thread that locked the mutex can unlock it. */


void internal_function __pthread_lock(struct _pthread_fastlock * lock,
				      pthread_descr self)
{
#if defined HAS_COMPARE_AND_SWAP
  long oldstatus, newstatus;
  int successful_seizure, spurious_wakeup_count = 0;
  int spin_count = 0;
#endif

#if defined TEST_FOR_COMPARE_AND_SWAP
  if (!__pthread_has_cas)
#endif
#if !defined HAS_COMPARE_AND_SWAP || defined TEST_FOR_COMPARE_AND_SWAP
  {
    __pthread_acquire(&lock->__spinlock);
    return;
  }
#endif

#if defined HAS_COMPARE_AND_SWAP
again:

  /* On SMP, try spinning to get the lock. */

  if (__pthread_smp_kernel) {
    int max_count = lock->__spinlock * 2 + 10;

    for (spin_count = 0; spin_count < max_count; spin_count++) {
      if (((oldstatus = lock->__status) & 1) == 0) {
	if(__compare_and_swap(&lock->__status, oldstatus, oldstatus | 1))
	{
	  if (spin_count)
	    lock->__spinlock += (spin_count - lock->__spinlock) / 8;
	  return;
	}
      }
    }

    lock->__spinlock += (spin_count - lock->__spinlock) / 8;
  }

  /* No luck, try once more or suspend. */

  do {
    oldstatus = lock->__status;
    successful_seizure = 0;

    if ((oldstatus & 1) == 0) {
      newstatus = oldstatus | 1;
      successful_seizure = 1;
    } else {
      if (self == NULL)
	self = thread_self();
      newstatus = (long) self | 1;
    }

    if (self != NULL) {
      THREAD_SETMEM(self, p_nextlock, (pthread_descr) (oldstatus & ~1L));
      /* Make sure the store in p_nextlock completes before performing
         the compare-and-swap */
      MEMORY_BARRIER();
    }
  } while(! __compare_and_swap(&lock->__status, oldstatus, newstatus));

  /* Suspend with guard against spurious wakeup.
     This can happen in pthread_cond_timedwait_relative, when the thread
     wakes up due to timeout and is still on the condvar queue, and then
     locks the queue to remove itself. At that point it may still be on the
     queue, and may be resumed by a condition signal. */

  if (!successful_seizure) {
    for (;;) {
      suspend(self);
      if (self->p_nextlock != NULL) {
	/* Count resumes that don't belong to us. */
	spurious_wakeup_count++;
	continue;
      }
      break;
    }
    goto again;
  }

  /* Put back any resumes we caught that don't belong to us. */
  while (spurious_wakeup_count--)
    restart(self);
#endif
}

int __pthread_unlock(struct _pthread_fastlock * lock)
{
#if defined HAS_COMPARE_AND_SWAP
  long oldstatus;
  pthread_descr thr, * ptr, * maxptr;
  int maxprio;
#endif

#if defined TEST_FOR_COMPARE_AND_SWAP
  if (!__pthread_has_cas)
#endif
#if !defined HAS_COMPARE_AND_SWAP || defined TEST_FOR_COMPARE_AND_SWAP
  {
    WRITE_MEMORY_BARRIER();
    lock->__spinlock = __LT_SPINLOCK_INIT;
    return 0;
  }
#endif

#if defined HAS_COMPARE_AND_SWAP
again:
  oldstatus = lock->__status;

  while ((oldstatus = lock->__status) == 1) {
    if (__compare_and_swap_with_release_semantics(&lock->__status,
	oldstatus, 0))
      return 0;
  }

  /* Find thread in waiting queue with maximal priority */
  ptr = (pthread_descr *) &lock->__status;
  thr = (pthread_descr) (oldstatus & ~1L);
  maxprio = 0;
  maxptr = ptr;
  while (thr != 0) {
    if (thr->p_priority >= maxprio) {
      maxptr = ptr;
      maxprio = thr->p_priority;
    }
    ptr = &(thr->p_nextlock);
    /* Prevent reordering of the load of lock->__status above and the
       load of *ptr below, as well as reordering of *ptr between
       several iterations of the while loop.  Some processors (e.g.
       multiprocessor Alphas) could perform such reordering even though
       the loads are dependent. */
    READ_MEMORY_BARRIER();
    thr = *ptr;
  }
  /* Prevent reordering of the load of lock->__status above and
     thr->p_nextlock below */
  READ_MEMORY_BARRIER();
  /* Remove max prio thread from waiting list. */
  if (maxptr == (pthread_descr *) &lock->__status) {
    /* If max prio thread is at head, remove it with compare-and-swap
       to guard against concurrent lock operation. This removal
       also has the side effect of marking the lock as released
       because the new status comes from thr->p_nextlock whose
       least significant bit is clear. */
    thr = (pthread_descr) (oldstatus & ~1L);
    if (! __compare_and_swap_with_release_semantics
	    (&lock->__status, oldstatus, (long)(thr->p_nextlock)))
      goto again;
  } else {
    /* No risk of concurrent access, remove max prio thread normally.
       But in this case we must also flip the least significant bit
       of the status to mark the lock as released. */
    thr = *maxptr;
    *maxptr = thr->p_nextlock;

    do {
      oldstatus = lock->__status;
    } while (!__compare_and_swap_with_release_semantics(&lock->__status,
	     oldstatus, oldstatus & ~1L));
  }
  /* Prevent reordering of store to *maxptr above and store to thr->p_nextlock
     below */
  WRITE_MEMORY_BARRIER();
  /* Wake up the selected waiting thread */
  thr->p_nextlock = NULL;
  restart(thr);

  return 0;
#endif
}

/*
 * Alternate fastlocks do not queue threads directly. Instead, they queue
 * these wait queue node structures. When a timed wait wakes up due to
 * a timeout, it can leave its wait node in the queue (because there
 * is no safe way to remove from the quue). Some other thread will
 * deallocate the abandoned node.
 */


struct wait_node {
  struct wait_node *next;	/* Next node in null terminated linked list */
  pthread_descr thr;		/* The thread waiting with this node */
  int abandoned;		/* Atomic flag */
};

static long wait_node_free_list;
#if !defined HAS_COMPARE_AND_SWAP || defined TEST_FOR_COMPARE_AND_SWAP
static int wait_node_free_list_spinlock;
#endif

/* Allocate a new node from the head of the free list using an atomic
   operation, or else using malloc if that list is empty.  A fundamental
   assumption here is that we can safely access wait_node_free_list->next.
   That's because we never free nodes once we allocate them, so a pointer to a
   node remains valid indefinitely. */

static struct wait_node *wait_node_alloc(void)
{
#if defined HAS_COMPARE_AND_SWAP
  long oldvalue, newvalue;
#endif

#if defined TEST_FOR_COMPARE_AND_SWAP
  if (!__pthread_has_cas)
#endif
#if !defined HAS_COMPARE_AND_SWAP || defined TEST_FOR_COMPARE_AND_SWAP
  {
    struct wait_node *new_node = 0;

    __pthread_acquire(&wait_node_free_list_spinlock);
    if (wait_node_free_list != 0) {
      new_node = (struct wait_node *) wait_node_free_list;
      wait_node_free_list = (long) new_node->next;
    }
    WRITE_MEMORY_BARRIER();
    wait_node_free_list_spinlock = 0;

    if (new_node == 0)
      return malloc(sizeof *wait_node_alloc());

    return new_node;
  }
#endif

#if defined HAS_COMPARE_AND_SWAP
  do {
    oldvalue = wait_node_free_list;

    if (oldvalue == 0)
      return malloc(sizeof *wait_node_alloc());

    newvalue = (long) ((struct wait_node *) oldvalue)->next;
    WRITE_MEMORY_BARRIER();
  } while (! __compare_and_swap(&wait_node_free_list, oldvalue, newvalue));

  return (struct wait_node *) oldvalue;
#endif
}

/* Return a node to the head of the free list using an atomic
   operation. */

static void wait_node_free(struct wait_node *wn)
{
#if defined HAS_COMPARE_AND_SWAP
  long oldvalue, newvalue;
#endif

#if defined TEST_FOR_COMPARE_AND_SWAP
  if (!__pthread_has_cas)
#endif
#if !defined HAS_COMPARE_AND_SWAP || defined TEST_FOR_COMPARE_AND_SWAP
  {
    __pthread_acquire(&wait_node_free_list_spinlock);
    wn->next = (struct wait_node *) wait_node_free_list;
    wait_node_free_list = (long) wn;
    WRITE_MEMORY_BARRIER();
    wait_node_free_list_spinlock = 0;
    return;
  }
#endif

#if defined HAS_COMPARE_AND_SWAP
  do {
    oldvalue = wait_node_free_list;
    wn->next = (struct wait_node *) oldvalue;
    newvalue = (long) wn;
    WRITE_MEMORY_BARRIER();
  } while (! __compare_and_swap(&wait_node_free_list, oldvalue, newvalue));
#endif
}

#if defined HAS_COMPARE_AND_SWAP

/* Remove a wait node from the specified queue.  It is assumed
   that the removal takes place concurrently with only atomic insertions at the
   head of the queue. */

static void wait_node_dequeue(struct wait_node **pp_head,
			      struct wait_node **pp_node,
			      struct wait_node *p_node)
{
  /* If the node is being deleted from the head of the
     list, it must be deleted using atomic compare-and-swap.
     Otherwise it can be deleted in the straightforward way. */

  if (pp_node == pp_head) {
    long oldvalue = (long) p_node;
    long newvalue = (long) p_node->next;

    if (__compare_and_swap((long *) pp_node, oldvalue, newvalue))
      return;

    /* Oops! Compare and swap failed, which means the node is
       no longer first. We delete it using the ordinary method.  But we don't
       know the identity of the node which now holds the pointer to the node
       being deleted, so we must search from the beginning. */

    for (pp_node = pp_head; *pp_node != p_node; pp_node = &(*pp_node)->next)
      ; /* null body */
  }

  *pp_node = p_node->next;
  return;
}

#endif

void __pthread_alt_lock(struct _pthread_fastlock * lock,
		        pthread_descr self)
{
#if defined HAS_COMPARE_AND_SWAP
  long oldstatus, newstatus;
#endif
  struct wait_node wait_node;

#if defined TEST_FOR_COMPARE_AND_SWAP
  if (!__pthread_has_cas)
#endif
#if !defined HAS_COMPARE_AND_SWAP || defined TEST_FOR_COMPARE_AND_SWAP
  {
    int suspend_needed = 0;
    __pthread_acquire(&lock->__spinlock);

    if (lock->__status == 0)
      lock->__status = 1;
    else {
      if (self == NULL)
	self = thread_self();

      wait_node.abandoned = 0;
      wait_node.next = (struct wait_node *) lock->__status;
      wait_node.thr = self;
      lock->__status = (long) &wait_node;
      suspend_needed = 1;
    }

    WRITE_MEMORY_BARRIER();
    lock->__spinlock = __LT_SPINLOCK_INIT;

    if (suspend_needed)
      suspend (self);
    return;
  }
#endif

#if defined HAS_COMPARE_AND_SWAP
  do {
    oldstatus = lock->__status;
    if (oldstatus == 0) {
      newstatus = 1;
    } else {
      if (self == NULL)
	self = thread_self();
      wait_node.thr = self;
      newstatus = (long) &wait_node;
    }
    wait_node.abandoned = 0;
    wait_node.next = (struct wait_node *) oldstatus;
    /* Make sure the store in wait_node.next completes before performing
       the compare-and-swap */
    MEMORY_BARRIER();
  } while(! __compare_and_swap(&lock->__status, oldstatus, newstatus));

  /* Suspend. Note that unlike in __pthread_lock, we don't worry
     here about spurious wakeup. That's because this lock is not
     used in situations where that can happen; the restart can
     only come from the previous lock owner. */

  if (oldstatus != 0)
    suspend(self);
#endif
}

/* Timed-out lock operation; returns 0 to indicate timeout. */

int __pthread_alt_timedlock(struct _pthread_fastlock * lock,
			    pthread_descr self, const struct timespec *abstime)
{
  long oldstatus = 0;
#if defined HAS_COMPARE_AND_SWAP
  long newstatus;
#endif
  struct wait_node *p_wait_node = wait_node_alloc();

  /* Out of memory, just give up and do ordinary lock. */
  if (p_wait_node == 0) {
    __pthread_alt_lock(lock, self);
    return 1;
  }

#if defined TEST_FOR_COMPARE_AND_SWAP
  if (!__pthread_has_cas)
#endif
#if !defined HAS_COMPARE_AND_SWAP || defined TEST_FOR_COMPARE_AND_SWAP
  {
    __pthread_acquire(&lock->__spinlock);

    if (lock->__status == 0)
      lock->__status = 1;
    else {
      if (self == NULL)
	self = thread_self();

      p_wait_node->abandoned = 0;
      p_wait_node->next = (struct wait_node *) lock->__status;
      p_wait_node->thr = self;
      lock->__status = (long) p_wait_node;
      oldstatus = 1; /* force suspend */
    }

    WRITE_MEMORY_BARRIER();
    lock->__spinlock = __LT_SPINLOCK_INIT;
    goto suspend;
  }
#endif

#if defined HAS_COMPARE_AND_SWAP
  do {
    oldstatus = lock->__status;
    if (oldstatus == 0) {
      newstatus = 1;
    } else {
      if (self == NULL)
	self = thread_self();
      p_wait_node->thr = self;
      newstatus = (long) p_wait_node;
    }
    p_wait_node->abandoned = 0;
    p_wait_node->next = (struct wait_node *) oldstatus;
    /* Make sure the store in wait_node.next completes before performing
       the compare-and-swap */
    MEMORY_BARRIER();
  } while(! __compare_and_swap(&lock->__status, oldstatus, newstatus));
#endif

#if !defined HAS_COMPARE_AND_SWAP || defined TEST_FOR_COMPARE_AND_SWAP
  suspend:
#endif

  /* If we did not get the lock, do a timed suspend. If we wake up due
     to a timeout, then there is a race; the old lock owner may try
     to remove us from the queue. This race is resolved by us and the owner
     doing an atomic testandset() to change the state of the wait node from 0
     to 1. If we succeed, then it's a timeout and we abandon the node in the
     queue. If we fail, it means the owner gave us the lock. */

  if (oldstatus != 0) {
    if (timedsuspend(self, abstime) == 0) {
      if (!testandset(&p_wait_node->abandoned))
	return 0; /* Timeout! */

      /* Eat oustanding resume from owner, otherwise wait_node_free() below
	 will race with owner's wait_node_dequeue(). */
      suspend(self);
    }
  }

  wait_node_free(p_wait_node);

  return 1; /* Got the lock! */
}

void __pthread_alt_unlock(struct _pthread_fastlock *lock)
{
  struct wait_node *p_node, **pp_node, *p_max_prio, **pp_max_prio;
  struct wait_node ** const pp_head = (struct wait_node **) &lock->__status;
  int maxprio;

#if defined TEST_FOR_COMPARE_AND_SWAP
  if (!__pthread_has_cas)
#endif
#if !defined HAS_COMPARE_AND_SWAP || defined TEST_FOR_COMPARE_AND_SWAP
  {
    __pthread_acquire(&lock->__spinlock);
  }
#endif

  while (1) {

  /* If no threads are waiting for this lock, try to just
     atomically release it. */
#if defined TEST_FOR_COMPARE_AND_SWAP
    if (!__pthread_has_cas)
#endif
#if !defined HAS_COMPARE_AND_SWAP || defined TEST_FOR_COMPARE_AND_SWAP
    {
      if (lock->__status == 0 || lock->__status == 1) {
	lock->__status = 0;
	break;
      }
    }
#endif

#if defined TEST_FOR_COMPARE_AND_SWAP
    else
#endif

#if defined HAS_COMPARE_AND_SWAP
    {
      long oldstatus = lock->__status;
      if (oldstatus == 0 || oldstatus == 1) {
	if (__compare_and_swap_with_release_semantics (&lock->__status, oldstatus, 0))
	  break;
	else
	  continue;
      }
    }
#endif

    /* Process the entire queue of wait nodes. Remove all abandoned
       wait nodes and put them into the global free queue, and
       remember the one unabandoned node which refers to the thread
       having the highest priority. */

    pp_max_prio = pp_node = pp_head;
    p_max_prio = p_node = *pp_head;
    maxprio = INT_MIN;

    while (p_node != (struct wait_node *) 1) {
      int prio;

      if (p_node->abandoned) {
	/* Remove abandoned node. */
#if defined TEST_FOR_COMPARE_AND_SWAP
	if (!__pthread_has_cas)
#endif
#if !defined HAS_COMPARE_AND_SWAP || defined TEST_FOR_COMPARE_AND_SWAP
	  *pp_node = p_node->next;
#endif
#if defined TEST_FOR_COMPARE_AND_SWAP
	else
#endif
#if defined HAS_COMPARE_AND_SWAP
	  wait_node_dequeue(pp_head, pp_node, p_node);
#endif
	wait_node_free(p_node);
	READ_MEMORY_BARRIER();
	p_node = *pp_node;
	continue;
      } else if ((prio = p_node->thr->p_priority) >= maxprio) {
	/* Otherwise remember it if its thread has a higher or equal priority
	   compared to that of any node seen thus far. */
	maxprio = prio;
	pp_max_prio = pp_node;
	p_max_prio = p_node;
      }

      pp_node = &p_node->next;
      READ_MEMORY_BARRIER();
      p_node = *pp_node;
    }

    READ_MEMORY_BARRIER();

    /* If all threads abandoned, go back to top */
    if (maxprio == INT_MIN)
      continue;

    ASSERT (p_max_prio != (struct wait_node *) 1);

    /* Now we want to to remove the max priority thread's wait node from
       the list. Before we can do this, we must atomically try to change the
       node's abandon state from zero to nonzero. If we succeed, that means we
       have the node that we will wake up. If we failed, then it means the
       thread timed out and abandoned the node in which case we repeat the
       whole unlock operation. */

    if (!testandset(&p_max_prio->abandoned)) {
#if defined TEST_FOR_COMPARE_AND_SWAP
      if (!__pthread_has_cas)
#endif
#if !defined HAS_COMPARE_AND_SWAP || defined TEST_FOR_COMPARE_AND_SWAP
	*pp_max_prio = p_max_prio->next;
#endif
#if defined TEST_FOR_COMPARE_AND_SWAP
      else
#endif
#if defined HAS_COMPARE_AND_SWAP
	wait_node_dequeue(pp_head, pp_max_prio, p_max_prio);
#endif
      WRITE_MEMORY_BARRIER();
      restart(p_max_prio->thr);
      break;
    }
  }

#if defined TEST_FOR_COMPARE_AND_SWAP
  if (!__pthread_has_cas)
#endif
#if !defined HAS_COMPARE_AND_SWAP || defined TEST_FOR_COMPARE_AND_SWAP
  {
    WRITE_MEMORY_BARRIER();
    lock->__spinlock = __LT_SPINLOCK_INIT;
  }
#endif
}


/* Compare-and-swap emulation with a spinlock */

#ifdef TEST_FOR_COMPARE_AND_SWAP
int __pthread_has_cas = 0;
#endif

#if !defined HAS_COMPARE_AND_SWAP || defined TEST_FOR_COMPARE_AND_SWAP

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
  /* Prevent reordering of store to *ptr above and store to *spinlock below */
  WRITE_MEMORY_BARRIER();
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
