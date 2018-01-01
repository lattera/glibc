/* Copyright (C) 2003-2018 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef _LOWLEVELLOCK_H
#define _LOWLEVELLOCK_H	1

#include <time.h>
#include <sys/param.h>
#include <bits/pthreadtypes.h>
#include <atomic.h>
#include <kernel-features.h>

#include <lowlevellock-futex.h>

static inline int
__attribute__ ((always_inline))
__lll_trylock (int *futex)
{
  return atomic_compare_and_exchange_val_24_acq (futex, 1, 0) != 0;
}
#define lll_trylock(futex) __lll_trylock (&(futex))

static inline int
__attribute__ ((always_inline))
__lll_cond_trylock (int *futex)
{
  return atomic_compare_and_exchange_val_24_acq (futex, 2, 0) != 0;
}
#define lll_cond_trylock(futex) __lll_cond_trylock (&(futex))


extern void __lll_lock_wait_private (int *futex) attribute_hidden;
extern void __lll_lock_wait (int *futex, int private) attribute_hidden;

static inline void
__attribute__ ((always_inline))
__lll_lock (int *futex, int private)
{
  int val = atomic_compare_and_exchange_val_24_acq (futex, 1, 0);

  if (__glibc_unlikely (val != 0))
    {
      if (__builtin_constant_p (private) && private == LLL_PRIVATE)
	__lll_lock_wait_private (futex);
      else
	__lll_lock_wait (futex, private);
    }
}
#define lll_lock(futex, private) __lll_lock (&(futex), private)

static inline void
__attribute__ ((always_inline))
__lll_cond_lock (int *futex, int private)
{
  int val = atomic_compare_and_exchange_val_24_acq (futex, 2, 0);

  if (__glibc_unlikely (val != 0))
    __lll_lock_wait (futex, private);
}
#define lll_cond_lock(futex, private) __lll_cond_lock (&(futex), private)


extern int __lll_timedlock_wait (int *futex, const struct timespec *,
				 int private) attribute_hidden;

static inline int
__attribute__ ((always_inline))
__lll_timedlock (int *futex, const struct timespec *abstime, int private)
{
  int val = atomic_compare_and_exchange_val_24_acq (futex, 1, 0);
  int result = 0;

  if (__glibc_unlikely (val != 0))
    result = __lll_timedlock_wait (futex, abstime, private);
  return result;
}
#define lll_timedlock(futex, abstime, private) \
  __lll_timedlock (&(futex), abstime, private)

#define lll_unlock(lock, private) \
  ((void) ({								      \
    int *__futex = &(lock);						      \
    int __private = (private);						      \
    int __val = atomic_exchange_24_rel (__futex, 0);			      \
    if (__glibc_unlikely (__val > 1))					      \
      lll_futex_wake (__futex, 1, __private);				      \
  }))

#define lll_islocked(futex) \
  (futex != 0)

/* Initializers for lock.  */
#define LLL_LOCK_INITIALIZER		(0)
#define LLL_LOCK_INITIALIZER_LOCKED	(1)

/* The kernel notifies a process which uses CLONE_CHILD_CLEARTID via futex
   wakeup when the clone terminates.  The memory location contains the
   thread ID while the clone is running and is reset to zero
   afterwards.	*/
#define lll_wait_tid(tid) \
  do							\
    {							\
      __typeof (tid) __tid;				\
      while ((__tid = (tid)) != 0)			\
	lll_futex_wait (&(tid), __tid, LLL_SHARED);	\
    }							\
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

#endif	/* lowlevellock.h */
