/* Copyright (C) 2003, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Paul Mackerras <paulus@au.ibm.com>, 2003.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Libr	\ary; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#ifndef _LOWLEVELLOCK_H
#define _LOWLEVELLOCK_H	1

#include <time.h>
#include <sys/param.h>
#include <bits/pthreadtypes.h>
#include <atomic.h>


#ifndef __NR_futex
# define __NR_futex		221
#endif
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
									      \
    __ret = INTERNAL_SYSCALL (futex, __err, 4,				      \
			      (futexp), FUTEX_WAIT, (val), 0);		      \
    INTERNAL_SYSCALL_ERROR_P (__ret, __err) ? -__ret : __ret;		      \
  })

#define lll_futex_timed_wait(futexp, val, timespec) \
  ({									      \
    INTERNAL_SYSCALL_DECL (__err);					      \
    long int __ret;							      \
									      \
    __ret = INTERNAL_SYSCALL (futex, __err, 4,				      \
			      (futexp), FUTEX_WAIT, (val), (timespec));	      \
    INTERNAL_SYSCALL_ERROR_P (__ret, __err) ? -__ret : __ret;		      \
  })

#define lll_futex_wake(futexp, nr) \
  ({									      \
    INTERNAL_SYSCALL_DECL (__err);					      \
    long int __ret;							      \
									      \
    __ret = INTERNAL_SYSCALL (futex, __err, 4,				      \
			      (futexp), FUTEX_WAKE, (nr), 0);		      \
    INTERNAL_SYSCALL_ERROR_P (__ret, __err) ? -__ret : __ret;		      \
  })

/* Returns non-zero if error happened, zero if success.  */
#define lll_futex_requeue(futexp, nr_wake, nr_move, mutex, val) \
  ({									      \
    INTERNAL_SYSCALL_DECL (__err);					      \
    long int __ret;							      \
									      \
    __ret = INTERNAL_SYSCALL (futex, __err, 6,				      \
			      (futexp), FUTEX_CMP_REQUEUE, (nr_wake),	      \
			      (nr_move), (mutex), (val));		      \
    INTERNAL_SYSCALL_ERROR_P (__ret, __err);				      \
  })

#ifdef UP
# define __lll_acq_instr	""
# define __lll_rel_instr	""
#else
# define __lll_acq_instr	"isync"
# define __lll_rel_instr	"sync"
#endif

/* Set *futex to 1 if it is 0, atomically.  Returns the old value */
#define __lll_trylock(futex) \
  ({ int __val;								      \
     __asm __volatile ("1:	lwarx	%0,0,%2\n"			      \
		       "	cmpwi	0,%0,0\n"			      \
		       "	bne	2f\n"				      \
		       "	stwcx.	%3,0,%2\n"			      \
		       "	bne-	1b\n"				      \
		       "2:	" __lll_acq_instr			      \
		       : "=&r" (__val), "=m" (*futex)			      \
		       : "r" (futex), "r" (1), "m" (*futex)		      \
		       : "cr0", "memory");				      \
     __val;								      \
  })

#define lll_mutex_trylock(lock)	__lll_trylock (&(lock))

/* Set *futex to 2 if it is 0, atomically.  Returns the old value */
#define __lll_cond_trylock(futex) \
  ({ int __val;								      \
     __asm __volatile ("1:	lwarx	%0,0,%2\n"			      \
		       "	cmpwi	0,%0,0\n"			      \
		       "	bne	2f\n"				      \
		       "	stwcx.	%3,0,%2\n"			      \
		       "	bne-	1b\n"				      \
		       "2:	" __lll_acq_instr			      \
		       : "=&r" (__val), "=m" (*futex)			      \
		       : "r" (futex), "r" (2), "m" (*futex)		      \
		       : "cr0", "memory");				      \
     __val;								      \
  })
#define lll_mutex_cond_trylock(lock)	__lll_cond_trylock (&(lock))


extern void __lll_lock_wait (int *futex) attribute_hidden;

#define lll_mutex_lock(lock) \
  (void) ({								      \
    int *__futex = &(lock);						      \
    if (__builtin_expect (atomic_compare_and_exchange_val_acq (__futex, 1, 0),\
			  0) != 0)					      \
      __lll_lock_wait (__futex);					      \
  })

#define lll_mutex_cond_lock(lock) \
  (void) ({								      \
    int *__futex = &(lock);						      \
    if (__builtin_expect (atomic_compare_and_exchange_val_acq (__futex, 2, 0),\
			  0) != 0)					      \
      __lll_lock_wait (__futex);					      \
  })

extern int __lll_timedlock_wait
  (int *futex, const struct timespec *) attribute_hidden;

#define lll_mutex_timedlock(lock, abstime) \
  ({									      \
    int *__futex = &(lock);						      \
    int __val = 0;							      \
    if (__builtin_expect (atomic_compare_and_exchange_val_acq (__futex, 1, 0),\
			  0) != 0)					      \
      __val = __lll_timedlock_wait (__futex, abstime);			      \
    __val;								      \
  })

#define lll_mutex_unlock(lock) \
  ((void) ({								      \
    int *__futex = &(lock);						      \
    int __val = atomic_exchange_rel (__futex, 0);			      \
    if (__builtin_expect (__val > 1, 0))				      \
      lll_futex_wake (__futex, 1);					      \
  }))

#define lll_mutex_unlock_force(lock) \
  ((void) ({								      \
    int *__futex = &(lock);						      \
    *__futex = 0;							      \
    __asm __volatile (__lll_rel_instr ::: "memory");			      \
    lll_futex_wake (__futex, 1);					      \
  }))

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
  do {									      \
    __typeof (tid) __tid;						      \
    while ((__tid = (tid)) != 0)					      \
      lll_futex_wait (&(tid), __tid);					      \
  } while (0)

extern int __lll_timedwait_tid (int *, const struct timespec *)
     attribute_hidden;

#define lll_timedwait_tid(tid, abstime) \
  ({									      \
    int __res = 0;							      \
    if ((tid) != 0)							      \
      __res = __lll_timedwait_tid (&(tid), (abstime));			      \
    __res;								      \
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
