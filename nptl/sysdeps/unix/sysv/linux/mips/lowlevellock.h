/* Copyright (C) 2003, 2004, 2005 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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
#include <atomic.h>
#include <sysdep.h>


#define FUTEX_WAIT		0
#define FUTEX_WAKE		1
#define FUTEX_REQUEUE		3
#define FUTEX_CMP_REQUEUE	4

/* Initializer for compatibility lock.	*/
#define LLL_MUTEX_LOCK_INITIALIZER (0)

#define lll_futex_wait(futexp, val) \
  ({									      \
    INTERNAL_SYSCALL_DECL (__err);					      \
    long int __ret;							      \
    __ret = INTERNAL_SYSCALL (futex, __err, 4,				      \
			      (futexp), FUTEX_WAIT, (val), 0);		      \
    INTERNAL_SYSCALL_ERROR_P (__ret, __err) ? -__ret : __ret;		      \
  })

#define lll_futex_timed_wait(futexp, val, timespec) \
  ({									      \
    INTERNAL_SYSCALL_DECL (__err);					      \
    long int __ret;							      \
    __ret = INTERNAL_SYSCALL (futex, __err, 4,				      \
			      (futexp), FUTEX_WAIT, (val), (timespec));	      \
    INTERNAL_SYSCALL_ERROR_P (__ret, __err) ? -__ret : __ret;		      \
  })

#define lll_futex_wake(futexp, nr) \
  ({									      \
    INTERNAL_SYSCALL_DECL (__err);					      \
    long int __ret;							      \
    __ret = INTERNAL_SYSCALL (futex, __err, 4,				      \
			      (futexp), FUTEX_WAKE, (nr), 0);		      \
    INTERNAL_SYSCALL_ERROR_P (__ret, __err) ? -__ret : __ret;		      \
  })

/* Returns non-zero if error happened, zero if success.  */
#define lll_futex_requeue(futexp, nr_wake, nr_move, mutex, val) \
  ({									      \
    INTERNAL_SYSCALL_DECL (__err);					      \
    long int __ret;							      \
    __ret = INTERNAL_SYSCALL (futex, __err, 6,				      \
			      (futexp), FUTEX_CMP_REQUEUE, (nr_wake),	      \
			      (nr_move), (mutex), (val));		      \
    INTERNAL_SYSCALL_ERROR_P (__ret, __err);				      \
  })


static inline int __attribute__((always_inline))
__lll_mutex_trylock(int *futex)
{
  return atomic_compare_and_exchange_val_acq (futex, 1, 0) != 0;
}
#define lll_mutex_trylock(lock)	__lll_mutex_trylock (&(lock))


static inline int __attribute__((always_inline))
__lll_mutex_cond_trylock(int *futex)
{
  return atomic_compare_and_exchange_val_acq (futex, 2, 0) != 0;
}
#define lll_mutex_cond_trylock(lock)	__lll_mutex_cond_trylock (&(lock))


extern void __lll_lock_wait (int *futex) attribute_hidden;

static inline void __attribute__((always_inline))
__lll_mutex_lock(int *futex)
{
  if (atomic_compare_and_exchange_bool_acq (futex, 1, 0) != 0)
    __lll_lock_wait (futex);
}
#define lll_mutex_lock(futex) __lll_mutex_lock (&(futex))


static inline void __attribute__ ((always_inline))
__lll_mutex_cond_lock (int *futex)
{
  if (atomic_compare_and_exchange_bool_acq (futex, 2, 0) != 0)
    __lll_lock_wait (futex);
}
#define lll_mutex_cond_lock(futex) __lll_mutex_cond_lock (&(futex))


extern int __lll_timedlock_wait (int *futex, const struct timespec *)
	attribute_hidden;

static inline int __attribute__ ((always_inline))
__lll_mutex_timedlock (int *futex, const struct timespec *abstime)
{
  int result = 0;
  if (atomic_compare_and_exchange_bool_acq (futex, 1, 0) != 0)
    result = __lll_timedlock_wait (futex, abstime);
  return result;
}
#define lll_mutex_timedlock(futex, abstime) \
  __lll_mutex_timedlock (&(futex), abstime)


static inline void __attribute__ ((always_inline))
__lll_mutex_unlock (int *futex)
{
  int val = atomic_exchange_rel (futex, 0);
  if (__builtin_expect (val > 1, 0))
    lll_futex_wake (futex, 1);
}
#define lll_mutex_unlock(futex) __lll_mutex_unlock(&(futex))


static inline void __attribute__ ((always_inline))
__lll_mutex_unlock_force (int *futex)
{
  (void) atomic_exchange_rel (futex, 0);
  lll_futex_wake (futex, 1);
}
#define lll_mutex_unlock_force(futex) __lll_mutex_unlock_force(&(futex))


#define lll_mutex_islocked(futex) \
  (futex != 0)


/* Our internal lock implementation is identical to the binary-compatible
   mutex implementation. */

/* Type for lock object.  */
typedef int lll_lock_t;

/* Initializers for lock.  */
#define LLL_LOCK_INITIALIZER		(0)
#define LLL_LOCK_INITIALIZER_LOCKED	(1)

extern int lll_unlock_wake_cb (int *__futex) attribute_hidden;

/* The states of a lock are:
    0  -  untaken
    1  -  taken by one user
   >1  -  taken by more users */

#define lll_trylock(lock)	lll_mutex_trylock (lock)
#define lll_lock(lock)		lll_mutex_lock (lock)
#define lll_unlock(lock)	lll_mutex_unlock (lock)
#define lll_islocked(lock)	lll_mutex_islocked (lock)

/* The kernel notifies a process which uses CLONE_CLEARTID via futex
   wakeup when the clone terminates.  The memory location contains the
   thread ID while the clone is running and is reset to zero
   afterwards.	*/
#define lll_wait_tid(tid) \
  do {					\
    __typeof (tid) __tid;		\
    while ((__tid = (tid)) != 0)	\
      lll_futex_wait (&(tid), __tid);	\
  } while (0)

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
