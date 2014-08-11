/* Low-level lock implementation.  Generic futex-based version.
   Copyright (C) 2005-2014 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef _LOWLEVELLOCK_H
#define _LOWLEVELLOCK_H	1

#include <atomic.h>
#include <lowlevellock-futex.h>

#define lll_trylock(lock)	\
  atomic_compare_and_exchange_bool_acq (&(lock), 1, 0)

#define lll_cond_trylock(lock)	\
  atomic_compare_and_exchange_bool_acq (&(lock), 2, 0)

extern void __lll_lock_wait_private (int *futex) attribute_hidden;
extern void __lll_lock_wait (int *futex, int private) attribute_hidden;
extern int __lll_robust_lock_wait (int *futex, int private) attribute_hidden;

/* This is an expression rather than a statement even though its value is
   void, so that it can be used in a comma expression or as an expression
   that's cast to void.  */
#define __lll_lock(futex, private)                                      \
  ((void)                                                               \
   ({                                                                   \
     int *__futex = (futex);                                            \
     if (__glibc_unlikely                                               \
         (atomic_compare_and_exchange_bool_acq (__futex, 1, 0)))        \
       {                                                                \
         if (__builtin_constant_p (private) && (private) == LLL_PRIVATE) \
           __lll_lock_wait_private (__futex);                           \
         else                                                           \
           __lll_lock_wait (__futex, private);                          \
       }                                                                \
   }))
#define lll_lock(futex, private)	\
  __lll_lock (&(futex), private)


#define __lll_robust_lock(futex, id, private)                           \
  ({                                                                    \
    int *__futex = (futex);                                             \
    int __val = 0;                                                      \
                                                                        \
    if (__glibc_unlikely                                                \
        (atomic_compare_and_exchange_bool_acq (__futex, id, 0)))        \
      __val = __lll_robust_lock_wait (__futex, private);                \
    __val;                                                              \
  })
#define lll_robust_lock(futex, id, private)     \
  __lll_robust_lock (&(futex), id, private)


/* This is an expression rather than a statement even though its value is
   void, so that it can be used in a comma expression or as an expression
   that's cast to void.  */
#define __lll_cond_lock(futex, private)                                 \
  ((void)                                                               \
   ({                                                                   \
     int *__futex = (futex);                                            \
     if (__glibc_unlikely (atomic_exchange_acq (__futex, 2) != 0))      \
       __lll_lock_wait (__futex, private);                              \
   }))
#define lll_cond_lock(futex, private) __lll_cond_lock (&(futex), private)


#define lll_robust_cond_lock(futex, id, private)	\
  __lll_robust_lock (&(futex), (id) | FUTEX_WAITERS, private)


extern int __lll_timedlock_wait (int *futex, const struct timespec *,
				 int private) attribute_hidden;
extern int __lll_robust_timedlock_wait (int *futex, const struct timespec *,
					int private) attribute_hidden;

/* Take futex if it is untaken.
   Otherwise block until either we get the futex or abstime runs out.  */
#define __lll_timedlock(futex, abstime, private)                \
  ({                                                            \
    int *__futex = (futex);                                     \
    int __val = 0;                                              \
                                                                \
    if (__glibc_unlikely                                        \
        (atomic_compare_and_exchange_bool_acq (__futex, 1, 0))) \
      __val = __lll_timedlock_wait (__futex, abstime, private); \
    __val;                                                      \
  })
#define lll_timedlock(futex, abstime, private)  \
  __lll_timedlock (&(futex), abstime, private)


#define __lll_robust_timedlock(futex, abstime, id, private)             \
  ({                                                                    \
    int *__futex = (futex);                                             \
    int __val = 0;                                                      \
                                                                        \
    if (__glibc_unlikely                                                \
        (atomic_compare_and_exchange_bool_acq (__futex, id, 0)))        \
      __val = __lll_robust_timedlock_wait (__futex, abstime, private);  \
    __val;                                                              \
  })
#define lll_robust_timedlock(futex, abstime, id, private)       \
  __lll_robust_timedlock (&(futex), abstime, id, private)


/* This is an expression rather than a statement even though its value is
   void, so that it can be used in a comma expression or as an expression
   that's cast to void.  */
#define __lll_unlock(futex, private)                    \
  ((void)                                               \
   ({                                                   \
     int *__futex = (futex);                            \
     int __oldval = atomic_exchange_rel (__futex, 0);   \
     if (__glibc_unlikely (__oldval > 1))               \
       lll_futex_wake (__futex, 1, private);            \
   }))
#define lll_unlock(futex, private)	\
  __lll_unlock (&(futex), private)


/* This is an expression rather than a statement even though its value is
   void, so that it can be used in a comma expression or as an expression
   that's cast to void.  */
#define __lll_robust_unlock(futex, private)             \
  ((void)                                               \
   ({                                                   \
     int *__futex = (futex);                            \
     int __oldval = atomic_exchange_rel (__futex, 0);   \
     if (__glibc_unlikely (__oldval & FUTEX_WAITERS))	\
       lll_futex_wake (__futex, 1, private);            \
   }))
#define lll_robust_unlock(futex, private)       \
  __lll_robust_unlock (&(futex), private)


#define lll_islocked(futex) \
  ((futex) != LLL_LOCK_INITIALIZER)


/* Our internal lock implementation is identical to the binary-compatible
   mutex implementation. */

/* Initializers for lock.  */
#define LLL_LOCK_INITIALIZER		(0)
#define LLL_LOCK_INITIALIZER_LOCKED	(1)

/* The states of a lock are:
    0  -  untaken
    1  -  taken by one user
   >1  -  taken by more users */

/* The kernel notifies a process which uses CLONE_CHILD_CLEARTID via futex
   wakeup when the clone terminates.  The memory location contains the
   thread ID while the clone is running and is reset to zero
   afterwards.	*/
#define lll_wait_tid(tid) \
  do {					\
    __typeof (tid) __tid;		\
    while ((__tid = (tid)) != 0)	\
      lll_futex_wait (&(tid), __tid, LLL_SHARED);\
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


#endif	/* lowlevellock.h */
