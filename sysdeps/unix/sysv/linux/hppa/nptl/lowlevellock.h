/* Copyright (C) 2003, 2004, 2005, 2007 Free Software Foundation, Inc.
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
#include <sysdep.h>
#include <atomic.h>
#include <kernel-features.h>	/* Need __ASSUME_PRIVATE_FUTEX.  */
#include <tls.h>		/* Need THREAD_*, and header.*.  */

/* HPPA only has one atomic read and modify memory operation,
   load and clear, so hppa uses a kernel helper routine to implement
   compare_and_exchange. See atomic.h for the userspace calling
   sequence.  */

#define FUTEX_WAIT		0
#define FUTEX_WAKE		1
#define FUTEX_REQUEUE		3
#define FUTEX_CMP_REQUEUE	4
#define FUTEX_WAKE_OP		5
#define FUTEX_OP_CLEAR_WAKE_IF_GT_ONE	((4 << 24) | 1)
#define FUTEX_LOCK_PI		6
#define FUTEX_UNLOCK_PI		7
#define FUTEX_TRYLOCK_PI	8
#define FUTEX_WAIT_BITSET	9
#define FUTEX_WAKE_BITSET	10
#define FUTEX_PRIVATE_FLAG	128
#define FUTEX_CLOCK_REALTIME	256

#define FUTEX_BITSET_MATCH_ANY	0xffffffff

/* Values for 'private' parameter of locking macros.  Yes, the
   definition seems to be backwards.  But it is not.  The bit will be
   reversed before passing to the system call.  */
#define LLL_PRIVATE	0
#define LLL_SHARED	FUTEX_PRIVATE_FLAG

/* Initialize locks to zero.  */
#define LLL_MUTEX_LOCK_INITIALIZER (0)

#if !defined NOT_IN_libc || defined IS_IN_rtld
/* In libc.so or ld.so all futexes are private.  */
# ifdef __ASSUME_PRIVATE_FUTEX
#  define __lll_private_flag(fl, private) \
  ((fl) | FUTEX_PRIVATE_FLAG)
# else
#  define __lll_private_flag(fl, private) \
  ((fl) | THREAD_GETMEM (THREAD_SELF, header.private_futex))
# endif
#else
# ifdef __ASSUME_PRIVATE_FUTEX
#  define __lll_private_flag(fl, private) \
  (((fl) | FUTEX_PRIVATE_FLAG) ^ (private))
# else
#  define __lll_private_flag(fl, private) \
  (__builtin_constant_p (private)					      \
   ? ((private) == 0							      \
      ? ((fl) | THREAD_GETMEM (THREAD_SELF, header.private_futex))	      \
      : (fl))								      \
   : ((fl) | (((private) ^ FUTEX_PRIVATE_FLAG)				      \
	      & THREAD_GETMEM (THREAD_SELF, header.private_futex))))
# endif	      
#endif

/* Type for lock object.  */
typedef int lll_lock_t;

#define lll_futex_wait(futexp, val, private) \
  lll_futex_timed_wait (futexp, val, 0, private)

#define lll_futex_timed_wait(futexp, val, timespec, private) \
  ({									\
    INTERNAL_SYSCALL_DECL (__err);					\
    long int __ret;							\
    __ret = INTERNAL_SYSCALL (futex, __err, 4, (futexp), 		\
			      __lll_private_flag (FUTEX_WAIT, private),	\
			      (val), (timespec));			\
    __ret;								\
  })

#define lll_futex_wake(futexp, nr, private) \
  ({									\
    INTERNAL_SYSCALL_DECL (__err);					\
    long int __ret;							\
    __ret = INTERNAL_SYSCALL (futex, __err, 4, (futexp),		\
			      __lll_private_flag (FUTEX_WAKE, private),	\
			      (nr), 0);					\
    __ret;								\
  })

#define lll_private_futex_wait(futex, val) \
  lll_private_futex_timed_wait (futex, val, NULL)

#ifdef __ASSUME_PRIVATE_FUTEX
# define lll_private_futex_timed_wait(futexp, val, timespec) \
  ({									      \
    INTERNAL_SYSCALL_DECL (__err);					      \
    long int __ret;							      \
    __ret = INTERNAL_SYSCALL (futex, __err, 4,				      \
			      (futexp), FUTEX_WAIT | FUTEX_PRIVATE_FLAG,      \
			      (val), (timespec));			      \
    __ret;								      \
  })

# define lll_private_futex_wake(futexp, nr) \
  ({									      \
    INTERNAL_SYSCALL_DECL (__err);					      \
    long int __ret;							      \
    __ret = INTERNAL_SYSCALL (futex, __err, 4,				      \
			      (futexp), FUTEX_WAKE | FUTEX_PRIVATE_FLAG,      \
			      (nr), 0);					      \
    __ret;								      \
  })

#else

# define lll_private_futex_timed_wait(futexp, val, timespec) \
  ({									      \
    INTERNAL_SYSCALL_DECL (__err);					      \
    long int __ret, __op;						      \
    __op = FUTEX_WAIT | THREAD_GETMEM (THREAD_SELF, header.private_futex);    \
    __ret = INTERNAL_SYSCALL (futex, __err, 4,				      \
			      (futexp), __op, (val), (timespec));	      \
    __ret;								      \
  })

# define lll_private_futex_wake(futexp, nr) \
  ({									      \
    INTERNAL_SYSCALL_DECL (__err);					      \
    long int __ret, __op;						      \
    __op = FUTEX_WAKE | THREAD_GETMEM (THREAD_SELF, header.private_futex);    \
    __ret = INTERNAL_SYSCALL (futex, __err, 4,				      \
			      (futexp), __op, (nr), 0);			      \
    __ret;								      \
  })
#endif

/* Returns non-zero if error happened, zero if success.  */
#define lll_futex_requeue(futexp, nr_wake, nr_move, mutex, val, private) \
  ({									      \
    INTERNAL_SYSCALL_DECL (__err);					      \
    long int __ret;							      \
    __ret = INTERNAL_SYSCALL (futex, __err, 6, (futexp), 		      \
			      __lll_private_flag (FUTEX_CMP_REQUEUE, private),\
			      (nr_wake), (nr_move), (mutex), (val));	      \
    __ret;								      \
  })

#define lll_robust_dead(futexv, private) \
  do									      \
    {									      \
      int *__futexp = &(futexv);					      \
      atomic_or (__futexp, FUTEX_OWNER_DIED);				      \
      lll_futex_wake (__futexp, 1, private);				      \
    }									      \
  while (0)

/* Returns non-zero if error happened, zero if success.  */
#define lll_futex_wake_unlock(futexp, nr_wake, nr_wake2, futexp2, private) \
  ({									   \
    INTERNAL_SYSCALL_DECL (__err);					   \
    long int __ret;							   \
    __ret = INTERNAL_SYSCALL (futex, __err, 6, (futexp),		   \
			      __lll_private_flag (FUTEX_WAKE_OP, private), \
			      (nr_wake), (nr_wake2), (futexp2),		   \
			      FUTEX_OP_CLEAR_WAKE_IF_GT_ONE);		   \
    __ret;								   \
  })

static inline int
__attribute__ ((always_inline))
__lll_robust_trylock (int *futex, int id)
{
  return atomic_compare_and_exchange_val_acq (futex, id, 0) != 0;
}
#define lll_robust_trylock(futex, id) \
  __lll_robust_trylock (&(futex), id)

static inline int
__attribute__ ((always_inline))
__lll_cond_trylock (int *futex)
{
  return atomic_compare_and_exchange_val_acq (futex, 2, 0) != 0;
}
#define lll_cond_trylock(futex) __lll_cond_trylock (&(futex))

static inline int
__attribute__ ((always_inline))
__lll_trylock (int *futex)
{
  return atomic_compare_and_exchange_val_acq (futex, 1, 0) != 0;
}
#define lll_trylock(futex) __lll_trylock (&(futex))

extern void __lll_lock_wait (lll_lock_t *futex, int private) attribute_hidden;
extern void __lll_lock_wait_private (lll_lock_t *futex) attribute_hidden;

static inline void __attribute__((always_inline))
__lll_mutex_lock(lll_lock_t *futex, int private)
{
  int val = atomic_compare_and_exchange_val_acq (futex, 1, 0);

  if (__builtin_expect (val != 0, 0))
    {
      if (__builtin_constant_p (private) && private == LLL_PRIVATE)
	__lll_lock_wait_private (futex);
      else
	__lll_lock_wait (futex, private);
    }
}
#define lll_mutex_lock(futex, private) __lll_mutex_lock (&(futex), private)
#define lll_lock(lock, private)	lll_mutex_lock (lock, private)

extern int __lll_robust_lock_wait (int *futex, int private) attribute_hidden;

static inline int
__attribute__ ((always_inline))
__lll_robust_lock (int *futex, int id, int private)
{
  int result = 0;
  if (atomic_compare_and_exchange_bool_acq (futex, id, 0) != 0)
    result = __lll_robust_lock_wait (futex, private);
  return result;
}
#define lll_robust_lock(futex, id, private) \
  __lll_robust_lock (&(futex), id, private)

#define lll_robust_cond_lock(futex, id, private) \
  __lll_robust_lock (&(futex), (id) | FUTEX_WAITERS, private)

static inline void
__attribute__ ((always_inline))
__lll_cond_lock (int *futex, int private)
{
  int val = atomic_compare_and_exchange_val_acq (futex, 2, 0);

  if (__builtin_expect (val != 0, 0))
    __lll_lock_wait (futex, private);
}
#define lll_cond_lock(futex, private) __lll_cond_lock (&(futex), private)

extern int __lll_timedlock_wait (lll_lock_t *futex, const struct timespec *, 
				 int private) attribute_hidden;
extern int __lll_robust_timedlock_wait (int *futex, const struct timespec *,
				 int private) attribute_hidden;

static inline int
__attribute__ ((always_inline))
__lll_timedlock (int *futex, const struct timespec *abstime, int private)
{
  int val = atomic_compare_and_exchange_val_acq (futex, 1, 0);
  int result = 0;

  if (__builtin_expect (val != 0, 0))
    result = __lll_timedlock_wait (futex, abstime, private);
  return result;
}
#define lll_timedlock(futex, abstime, private) \
  __lll_timedlock (&(futex), abstime, private)

static inline int __attribute__ ((always_inline))
__lll_robust_timedlock (int *futex, const struct timespec *abstime,
			      int id, int private)
{
  int result = 0;
  if (atomic_compare_and_exchange_bool_acq (futex, id, 0) != 0)
    result = __lll_robust_timedlock_wait (futex, abstime, private);
  return result;
}
#define lll_robust_timedlock(futex, abstime, id, private) \
  __lll_robust_timedlock (&(futex), abstime, id, private)

#define __lll_unlock(futex, private) \
  (void)					\
  ({ int val = atomic_exchange_rel (futex, 0);  \
     if (__builtin_expect (val > 1, 0))         \
       lll_futex_wake (futex, 1, private);      \
  })
#define lll_unlock(futex, private) __lll_unlock(&(futex), private)

#define  __lll_robust_unlock(futex,private) \
  (void)                                               \
    ({ int val = atomic_exchange_rel (futex, 0);       \
       if (__builtin_expect (val & FUTEX_WAITERS, 0))  \
         lll_futex_wake (futex, 1, private);           \
    })
#define lll_robust_unlock(futex, private) \
  __lll_robust_unlock(&(futex), private)

#define lll_islocked(futex) \
  (futex != 0)

/* Our internal lock implementation is identical to the binary-compatible
   mutex implementation.  */
#define LLL_LOCK_INITIALIZER (0)
#define LLL_LOCK_INITIALIZER_CONST (0)
#define LLL_LOCK_INITIALIZER_LOCKED (1)

#define THREAD_INIT_LOCK(PD, LOCK) \
  (PD)->LOCK = LLL_LOCK_INITIALIZER

extern int lll_unlock_wake_cb (lll_lock_t *__futex) attribute_hidden;

/* The kernel notifies a process which uses CLONE_CLEARTID via futex
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
