/* Copyright (C) 2003, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Jakub Jelinek <jakub@redhat.com>, 2003.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#ifndef _LOWLEVELLOCK_H
#define _LOWLEVELLOCK_H	1

#include <time.h>
#include <sys/param.h>
#include <bits/pthreadtypes.h>
#include <ia64intrin.h>
#include <atomic.h>

#define __NR_futex		1230
#define FUTEX_WAIT		0
#define FUTEX_WAKE		1
#define FUTEX_REQUEUE		3
#define FUTEX_CMP_REQUEUE	4

/* Delay in spinlock loop.  */
#define BUSY_WAIT_NOP          asm ("hint @pause")

/* Initializer for compatibility lock.	*/
#define LLL_MUTEX_LOCK_INITIALIZER (0)

#define lll_futex_wait(futex, val) lll_futex_timed_wait (futex, val, 0)

#define lll_futex_timed_wait(ftx, val, timespec)			\
({									\
   DO_INLINE_SYSCALL(futex, 4, (long) (ftx), FUTEX_WAIT, (int) (val),	\
		     (long) (timespec));				\
   _r10 == -1 ? -_retval : _retval;					\
})

#define lll_futex_wake(ftx, nr)						\
({									\
   DO_INLINE_SYSCALL(futex, 3, (long) (ftx), FUTEX_WAKE, (int) (nr));	\
   _r10 == -1 ? -_retval : _retval;					\
})

/* Returns non-zero if error happened, zero if success.  */
#define lll_futex_requeue(ftx, nr_wake, nr_move, mutex, val)		     \
({									     \
   DO_INLINE_SYSCALL(futex, 6, (long) (ftx), FUTEX_CMP_REQUEUE,		     \
		     (int) (nr_wake), (int) (nr_move), (long) (mutex),	     \
		     (int) val);					     \
   _r10 == -1;								     \
})


#define __lll_mutex_trylock(futex) \
  (atomic_compare_and_exchange_val_acq (futex, 1, 0) != 0)
#define lll_mutex_trylock(futex) __lll_mutex_trylock (&(futex))


#define __lll_mutex_cond_trylock(futex) \
  (atomic_compare_and_exchange_val_acq (futex, 2, 0) != 0)
#define lll_mutex_cond_trylock(futex) __lll_mutex_cond_trylock (&(futex))


extern void __lll_lock_wait (int *futex) attribute_hidden;


#define __lll_mutex_lock(futex)						\
  ((void) ({								\
    int *__futex = (futex);						\
    if (atomic_compare_and_exchange_bool_acq (__futex, 1, 0) != 0)	\
      __lll_lock_wait (__futex);					\
  }))
#define lll_mutex_lock(futex) __lll_mutex_lock (&(futex))


#define __lll_mutex_cond_lock(futex)					\
  ((void) ({								\
    int *__futex = (futex);						\
    if (atomic_compare_and_exchange_bool_acq (__futex, 2, 0) != 0)	\
      __lll_lock_wait (__futex);					\
  }))
#define lll_mutex_cond_lock(futex) __lll_mutex_cond_lock (&(futex))


extern int __lll_timedlock_wait (int *futex, const struct timespec *)
     attribute_hidden;


#define __lll_mutex_timedlock(futex, abstime)				\
  ({									\
     int *__futex = (futex);						\
     int __val = 0;							\
									\
     if (atomic_compare_and_exchange_bool_acq (__futex, 1, 0) != 0)	\
       __val = __lll_timedlock_wait (__futex, abstime);			\
     __val;								\
  })
#define lll_mutex_timedlock(futex, abstime) \
  __lll_mutex_timedlock (&(futex), abstime)


#define __lll_mutex_unlock(futex)			\
  ((void) ({						\
    int *__futex = (futex);				\
    int __val = atomic_exchange_rel (__futex, 0);	\
							\
    if (__builtin_expect (__val > 1, 0))		\
      lll_futex_wake (__futex, 1);			\
  }))
#define lll_mutex_unlock(futex) \
  __lll_mutex_unlock(&(futex))


#define __lll_mutex_unlock_force(futex)		\
  ((void) ({					\
    int *__futex = (futex);			\
    (void) atomic_exchange_rel (__futex, 0);	\
    lll_futex_wake (__futex, 1);		\
  }))
#define lll_mutex_unlock_force(futex) \
  __lll_mutex_unlock_force(&(futex))


#define lll_mutex_islocked(futex) \
  (futex != 0)


/* We have a separate internal lock implementation which is not tied
   to binary compatibility.  We can use the lll_mutex_*.  */

/* Type for lock object.  */
typedef int lll_lock_t;

extern int lll_unlock_wake_cb (int *__futex) attribute_hidden;

/* Initializers for lock.  */
#define LLL_LOCK_INITIALIZER		(0)
#define LLL_LOCK_INITIALIZER_LOCKED	(1)

#define lll_trylock(futex)	lll_mutex_trylock (futex)
#define lll_lock(futex)		lll_mutex_lock (futex)
#define lll_unlock(futex)	lll_mutex_unlock (futex)
#define lll_islocked(futex)	lll_mutex_islocked (futex)


/* The kernel notifies a process with uses CLONE_CLEARTID via futex
   wakeup when the clone terminates.  The memory location contains the
   thread ID while the clone is running and is reset to zero
   afterwards.	*/
#define lll_wait_tid(tid) \
  do						\
    {						\
      __typeof (tid) __tid;			\
      while ((__tid = (tid)) != 0)		\
	lll_futex_wait (&(tid), __tid);		\
    }						\
  while (0)

extern int __lll_timedwait_tid (int *, const struct timespec *)
     attribute_hidden;

#define lll_timedwait_tid(tid, abstime) \
  ({							\
    int __res = 0;					\
    if ((tid) != 0)					\
      __res = __lll_timedwait_tid (&(tid), (abstime));	\
    __res;						\
  })


/* Conditional variable handling.  */

extern void __lll_cond_wait (pthread_cond_t *cond)
     attribute_hidden;
extern int __lll_cond_timedwait (pthread_cond_t *cond,
				 const struct timespec *abstime)
     attribute_hidden;
extern void __lll_cond_wake (pthread_cond_t *cond)
     attribute_hidden;
extern void __lll_cond_broadcast (pthread_cond_t *cond)
     attribute_hidden;

#define lll_cond_wait(cond) \
  __lll_cond_wait (cond)
#define lll_cond_timedwait(cond, abstime) \
  __lll_cond_timedwait (cond, abstime)
#define lll_cond_wake(cond) \
  __lll_cond_wake (cond)
#define lll_cond_broadcast(cond) \
  __lll_cond_broadcast (cond)

#endif	/* lowlevellock.h */
