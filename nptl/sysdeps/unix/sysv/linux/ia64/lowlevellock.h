/* Copyright (C) 2003 Free Software Foundation, Inc.
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

#define SYS_futex		1230
#define FUTEX_WAIT		0
#define FUTEX_WAKE		1

/* Initializer for compatibility lock.	*/
#define LLL_MUTEX_LOCK_INITIALIZER (0)

#define lll_futex_clobbers \
  "out4", "out5", "out6", "out7",					      \
  /* Non-stacked integer registers, minus r8, r10, r15.  */		      \
  "r2", "r3", "r9", "r11", "r12", "r13", "r14", "r16", "r17", "r18",	      \
  "r19", "r20", "r21", "r22", "r23", "r24", "r25", "r26", "r27",	      \
  "r28", "r29", "r30", "r31",						      \
  /* Predicate registers.  */						      \
  "p6", "p7", "p8", "p9", "p10", "p11", "p12", "p13", "p14", "p15",	      \
  /* Non-rotating fp registers.  */					      \
  "f6", "f7", "f8", "f9", "f10", "f11", "f12", "f13", "f14", "f15",	      \
  /* Branch registers.  */						      \
  "b6", "b7",								      \
  "memory"

#define lll_futex_wait(futex, val) lll_futex_timed_wait (futex, val, 0)

#define lll_futex_timed_wait(futex, val, timespec) \
  ({									      \
     register long int __o0 asm ("out0") = (long int) (futex);		      \
     register long int __o1 asm ("out1") = FUTEX_WAIT;			      \
     register long int __o2 asm ("out2") = (long int) (val);		      \
     register long int __o3 asm ("out3") = (long int) (timespec);	      \
     register long int __r8 asm ("r8");					      \
     register long int __r10 asm ("r10");				      \
     register long int __r15 asm ("r15") = SYS_futex;			      \
									      \
     __asm __volatile ("break %7;;"					      \
		       : "=r" (__r8), "=r" (__r10), "=r" (__r15),	      \
			 "=r" (__o0), "=r" (__o1), "=r" (__o2), "=r" (__o3)   \
		       : "i" (0x100000), "2" (__r15), "3" (__o0), "4" (__o1), \
		 	 "5" (__o2), "6" (__o3)				      \
		       : lll_futex_clobbers);				      \
     __r10 == -1 ? -__r8 : __r8;					      \
  })


#define lll_futex_wake(futex, nr) \
  ({									      \
     register long int __o0 asm ("out0") = (long int) (futex);		      \
     register long int __o1 asm ("out1") = FUTEX_WAKE;			      \
     register long int __o2 asm ("out2") = (long int) (nr);		      \
     register long int __r8 asm ("r8");					      \
     register long int __r10 asm ("r10");				      \
     register long int __r15 asm ("r15") = SYS_futex;			      \
									      \
     __asm __volatile ("break %6;;"					      \
		       : "=r" (__r8), "=r" (__r10), "=r" (__r15),	      \
			 "=r" (__o0), "=r" (__o1), "=r" (__o2)		      \
		       : "i" (0x100000), "2" (__r15), "3" (__o0), "4" (__o1), \
		 	 "5" (__o2)					      \
		       : "out3", lll_futex_clobbers);			      \
     __r10 == -1 ? -__r8 : __r8;					      \
  })

#define lll_compare_and_swap(futex, oldval, newval) \
  __sync_val_compare_and_swap_si ((futex), (oldval), (newval))

static inline int
__attribute__ ((always_inline))
__lll_mutex_trylock (int *futex)
{
  return lll_compare_and_swap (futex, 0, 1) != 0;
}
#define lll_mutex_trylock(futex) __lll_mutex_trylock (&(futex))


extern void ___lll_mutex_lock (int *, int) attribute_hidden;


static inline void
__attribute__ ((always_inline))
__lll_mutex_lock (int *futex)
{
  int oldval;
  int val = *futex;

  do
    oldval = val;
  while ((val = lll_compare_and_swap (futex, oldval, oldval + 1)) != oldval);
  if (oldval > 0)
    ___lll_mutex_lock (futex, oldval + 1);
}
#define lll_mutex_lock(futex) __lll_mutex_lock (&(futex))


extern int ___lll_mutex_timedlock (int *, const struct timespec *, int)
  attribute_hidden;


static inline int
__attribute__ ((always_inline))
__lll_mutex_timedlock (int *futex, const struct timespec *abstime)
{
  int oldval;
  int val = *futex;
  int result = 0;

  do
    oldval = val;
  while ((val = lll_compare_and_swap (futex, oldval, oldval + 1)) != oldval);
  if (oldval > 0)
    result = ___lll_mutex_timedlock (futex, abstime, oldval + 1);

  return result;
}
#define lll_mutex_timedlock(futex, abstime) \
  __lll_mutex_timedlock (&(futex), abstime)


static inline void
__attribute__ ((always_inline))
__lll_mutex_unlock (int *futex)
{
  int oldval;
  int val = *futex;

  do
    oldval = val;
  while ((val = lll_compare_and_swap (futex, oldval, 0)) != oldval);
  if (oldval > 1)
    lll_futex_wake (futex, 1);
}
#define lll_mutex_unlock(futex) __lll_mutex_unlock(&(futex))

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
static inline void
__attribute__ ((always_inline))
__lll_wait_tid (int *ptid)
{
  int tid;

  while ((tid = *ptid) != 0)
    lll_futex_wait (ptid, tid);
}
#define lll_wait_tid(tid) __lll_wait_tid(&(tid))


extern int ___lll_timedwait_tid (int *, const struct timespec *)
     attribute_hidden;
static inline int
__attribute__ ((always_inline))
__lll_timedwait_tid (int *ptid, const struct timespec *abstime)
{
  if (*ptid == 0)
    return 0;

  return ___lll_timedwait_tid (ptid, abstime);
}
#define lll_timedwait_tid(tid, abstime) __lll_timedwait_tid (&(tid), abstime)


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
