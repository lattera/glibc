/* Copyright (C) 2003, 2004, 2006, 2007, 2008, 2009
   Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef _LOWLEVELLOCK_H
#define _LOWLEVELLOCK_H	1

#ifndef __ASSEMBLER__
#include <time.h>
#include <sys/param.h>
#include <bits/pthreadtypes.h>
#include <kernel-features.h>
#endif

#define SYS_futex		240
#define FUTEX_WAIT		0
#define FUTEX_WAKE		1
#define FUTEX_CMP_REQUEUE	4
#define FUTEX_WAKE_OP		5
#define FUTEX_LOCK_PI		6
#define FUTEX_UNLOCK_PI		7
#define FUTEX_TRYLOCK_PI	8
#define FUTEX_WAIT_BITSET	9
#define FUTEX_WAKE_BITSET	10
#define FUTEX_PRIVATE_FLAG	128
#define FUTEX_CLOCK_REALTIME	256

#define FUTEX_BITSET_MATCH_ANY	0xffffffff

#define FUTEX_OP_CLEAR_WAKE_IF_GT_ONE	((4 << 24) | 1)

/* Values for 'private' parameter of locking macros.  Yes, the
   definition seems to be backwards.  But it is not.  The bit will be
   reversed before passing to the system call.  */
#define LLL_PRIVATE    0
#define LLL_SHARED     FUTEX_PRIVATE_FLAG


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

#ifndef __ASSEMBLER__

/* Initializer for compatibility lock.  */
#define LLL_LOCK_INITIALIZER		(0)
#define LLL_LOCK_INITIALIZER_LOCKED	(1)
#define LLL_LOCK_INITIALIZER_WAITERS	(2)

extern int __lll_lock_wait_private (int val, int *__futex)
  attribute_hidden;
extern int __lll_lock_wait (int val, int *__futex, int private)
  attribute_hidden;
extern int __lll_timedlock_wait (int val, int *__futex,
				 const struct timespec *abstime, int private)
  attribute_hidden;
extern int __lll_robust_lock_wait (int val, int *__futex, int private)
  attribute_hidden;
extern int __lll_robust_timedlock_wait (int val, int *__futex,
					const struct timespec *abstime,
					int private)
  attribute_hidden;
extern int __lll_unlock_wake_private (int *__futex) attribute_hidden;
extern int __lll_unlock_wake (int *__futex, int private) attribute_hidden;

#define lll_trylock(futex) \
  ({ unsigned char __result; \
     __asm __volatile ("\
	.align 2\n\
	mova 1f,r0\n\
	nop\n\
	mov r15,r1\n\
	mov #-8,r15\n\
     0: mov.l @%1,r2\n\
	cmp/eq r2,%3\n\
	bf 1f\n\
	mov.l %2,@%1\n\
     1: mov r1,r15\n\
	mov #-1,%0\n\
	negc %0,%0"\
	: "=r" (__result) \
	: "r" (&(futex)), \
	  "r" (LLL_LOCK_INITIALIZER_LOCKED), \
	  "r" (LLL_LOCK_INITIALIZER) \
	: "r0", "r1", "r2", "t", "memory"); \
     __result; })

#define lll_robust_trylock(futex, id)	\
  ({ unsigned char __result; \
     __asm __volatile ("\
	.align 2\n\
	mova 1f,r0\n\
	nop\n\
	mov r15,r1\n\
	mov #-8,r15\n\
     0: mov.l @%1,r2\n\
	cmp/eq r2,%3\n\
	bf 1f\n\
	mov.l %2,@%1\n\
     1: mov r1,r15\n\
	mov #-1,%0\n\
	negc %0,%0"\
	: "=r" (__result) \
	: "r" (&(futex)), \
	  "r" (id), \
	  "r" (LLL_LOCK_INITIALIZER) \
	: "r0", "r1", "r2", "t", "memory"); \
     __result; })

#define lll_cond_trylock(futex) \
  ({ unsigned char __result; \
     __asm __volatile ("\
	.align 2\n\
	mova 1f,r0\n\
	nop\n\
	mov r15,r1\n\
	mov #-8,r15\n\
     0: mov.l @%1,r2\n\
	cmp/eq r2,%3\n\
	bf 1f\n\
	mov.l %2,@%1\n\
     1: mov r1,r15\n\
	mov #-1,%0\n\
	negc %0,%0"\
	: "=r" (__result) \
	: "r" (&(futex)), \
	  "r" (LLL_LOCK_INITIALIZER_WAITERS), \
	  "r" (LLL_LOCK_INITIALIZER) \
	: "r0", "r1", "r2", "t", "memory"); \
     __result; })

#define lll_lock(futex, private) \
  (void) ({ int __result, *__futex = &(futex); \
	    __asm __volatile ("\
		.align 2\n\
		mova 1f,r0\n\
		nop\n\
		mov r15,r1\n\
		mov #-8,r15\n\
	     0: mov.l @%2,%0\n\
		tst %0,%0\n\
		bf 1f\n\
		mov.l %1,@%2\n\
	     1: mov r1,r15"\
		: "=&r" (__result) : "r" (1), "r" (__futex) \
		: "r0", "r1", "t", "memory"); \
	    if (__result) \
	      { \
		if (__builtin_constant_p (private) \
		    && (private) == LLL_PRIVATE) \
		  __lll_lock_wait_private (__result, __futex); \
	        else \
		  __lll_lock_wait (__result, __futex, (private));	\
	      } \
    })

#define lll_robust_lock(futex, id, private) \
  ({ int __result, *__futex = &(futex); \
     __asm __volatile ("\
	.align 2\n\
	mova 1f,r0\n\
	nop\n\
	mov r15,r1\n\
	mov #-8,r15\n\
      0: mov.l @%2,%0\n\
	tst %0,%0\n\
	bf 1f\n\
	mov.l %1,@%2\n\
      1: mov r1,r15"\
	: "=&r" (__result) : "r" (id), "r" (__futex) \
	: "r0", "r1", "t", "memory"); \
     if (__result) \
       __result = __lll_robust_lock_wait (__result, __futex, private); \
     __result; })

/* Special version of lll_mutex_lock which causes the unlock function to
   always wakeup waiters.  */
#define lll_cond_lock(futex, private) \
  (void) ({ int __result, *__futex = &(futex); \
	    __asm __volatile ("\
		.align 2\n\
		mova 1f,r0\n\
		nop\n\
		mov r15,r1\n\
		mov #-8,r15\n\
	     0: mov.l @%2,%0\n\
		tst %0,%0\n\
		bf 1f\n\
		mov.l %1,@%2\n\
	     1: mov r1,r15"\
		: "=&r" (__result) : "r" (2), "r" (__futex) \
		: "r0", "r1", "t", "memory"); \
	    if (__result) \
	      __lll_lock_wait (__result, __futex, private); })

#define lll_robust_cond_lock(futex, id, private) \
  ({ int __result, *__futex = &(futex); \
     __asm __volatile ("\
	.align 2\n\
	mova 1f,r0\n\
	nop\n\
	mov r15,r1\n\
	mov #-8,r15\n\
     0: mov.l @%2,%0\n\
	tst %0,%0\n\
	bf 1f\n\
	mov.l %1,@%2\n\
     1: mov r1,r15"\
	: "=&r" (__result) : "r" (id | FUTEX_WAITERS), "r" (__futex) \
	: "r0", "r1", "t", "memory"); \
      if (__result) \
	__result = __lll_robust_lock_wait (__result, __futex, private); \
      __result; })

#define lll_timedlock(futex, timeout, private) \
  ({ int __result, *__futex = &(futex); \
     __asm __volatile ("\
	.align 2\n\
	mova 1f,r0\n\
	nop\n\
	mov r15,r1\n\
	mov #-8,r15\n\
     0: mov.l @%2,%0\n\
	tst %0,%0\n\
	bf 1f\n\
	mov.l %1,@%2\n\
     1: mov r1,r15"\
	: "=&r" (__result) : "r" (1), "r" (__futex) \
	: "r0", "r1", "t", "memory"); \
    if (__result) \
      __result = __lll_timedlock_wait (__result, __futex, timeout, private); \
    __result; })

#define lll_robust_timedlock(futex, timeout, id, private) \
  ({ int __result, *__futex = &(futex); \
     __asm __volatile ("\
	.align 2\n\
	mova 1f,r0\n\
	nop\n\
	mov r15,r1\n\
	mov #-8,r15\n\
     0: mov.l @%2,%0\n\
	tst %0,%0\n\
	bf 1f\n\
	mov.l %1,@%2\n\
     1: mov r1,r15"\
	: "=&r" (__result) : "r" (id), "r" (__futex) \
	: "r0", "r1", "t", "memory"); \
    if (__result) \
      __result = __lll_robust_timedlock_wait (__result, __futex, \
					      timeout, private); \
    __result; })

#define lll_unlock(futex, private) \
  (void) ({ int __result, *__futex = &(futex); \
	    __asm __volatile ("\
		.align 2\n\
		mova 1f,r0\n\
		mov r15,r1\n\
		mov #-6,r15\n\
	     0: mov.l @%1,%0\n\
		add #-1,%0\n\
		mov.l %0,@%1\n\
	     1: mov r1,r15"\
		: "=&r" (__result) : "r" (__futex) \
		: "r0", "r1", "memory"); \
	    if (__result) \
	      { \
		if (__builtin_constant_p (private) \
		    && (private) == LLL_PRIVATE) \
		  __lll_unlock_wake_private (__futex); \
	        else \
		  __lll_unlock_wake (__futex, (private)); \
	      } \
    })

#define lll_robust_unlock(futex, private) \
  (void) ({ int __result, *__futex = &(futex); \
	    __asm __volatile ("\
		.align 2\n\
		mova 1f,r0\n\
		mov r15,r1\n\
		mov #-6,r15\n\
	     0: mov.l @%1,%0\n\
		and %2,%0\n\
		mov.l %0,@%1\n\
	     1: mov r1,r15"\
		: "=&r" (__result) : "r" (__futex), "r" (FUTEX_WAITERS) \
		: "r0", "r1", "memory");	\
	    if (__result) \
	      __lll_unlock_wake (__futex, private); })

#define lll_robust_dead(futex, private)		       \
  (void) ({ int __ignore, *__futex = &(futex); \
	    __asm __volatile ("\
		.align 2\n\
		mova 1f,r0\n\
		mov r15,r1\n\
		mov #-6,r15\n\
	     0: mov.l @%1,%0\n\
		or %2,%0\n\
		mov.l %0,@%1\n\
	     1: mov r1,r15"\
		: "=&r" (__ignore) : "r" (__futex), "r" (FUTEX_OWNER_DIED) \
		: "r0", "r1", "memory");	\
	    lll_futex_wake (__futex, 1, private); })

# ifdef NEED_SYSCALL_INST_PAD
#  define SYSCALL_WITH_INST_PAD "\
	trapa #0x14; or r0,r0; or r0,r0; or r0,r0; or r0,r0; or r0,r0"
# else
#  define SYSCALL_WITH_INST_PAD "\
	trapa #0x14"
# endif

#define lll_futex_wait(futex, val, private) \
  lll_futex_timed_wait (futex, val, NULL, private)


#define lll_futex_timed_wait(futex, val, timeout, private) \
  ({									      \
    int __status;							      \
    register unsigned long __r3 asm ("r3") = SYS_futex;			      \
    register unsigned long __r4 asm ("r4") = (unsigned long) (futex);	      \
    register unsigned long __r5 asm ("r5")				      \
      = __lll_private_flag (FUTEX_WAIT, private);			      \
    register unsigned long __r6 asm ("r6") = (unsigned long) (val);	      \
    register unsigned long __r7 asm ("r7") = (timeout);			      \
    __asm __volatile (SYSCALL_WITH_INST_PAD				      \
		      : "=z" (__status)					      \
		      : "r" (__r3), "r" (__r4), "r" (__r5),		      \
			"r" (__r6), "r" (__r7)				      \
		      : "memory", "t");					      \
    __status;								      \
  })


#define lll_futex_wake(futex, nr, private) \
  do {									      \
    int __ignore;							      \
    register unsigned long __r3 asm ("r3") = SYS_futex;			      \
    register unsigned long __r4 asm ("r4") = (unsigned long) (futex);	      \
    register unsigned long __r5 asm ("r5")				      \
      = __lll_private_flag (FUTEX_WAKE, private);			      \
    register unsigned long __r6 asm ("r6") = (unsigned long) (nr);	      \
    register unsigned long __r7 asm ("r7") = 0;				      \
    __asm __volatile (SYSCALL_WITH_INST_PAD				      \
		      : "=z" (__ignore)					      \
		      : "r" (__r3), "r" (__r4), "r" (__r5),		      \
			"r" (__r6), "r" (__r7)				      \
		      : "memory", "t");					      \
  } while (0)


#define lll_islocked(futex) \
  (futex != LLL_LOCK_INITIALIZER)

/* The kernel notifies a process with uses CLONE_CLEARTID via futex
   wakeup when the clone terminates.  The memory location contains the
   thread ID while the clone is running and is reset to zero
   afterwards.  */

#define lll_wait_tid(tid) \
  do {									      \
    __typeof (tid) *__tid = &(tid);					      \
    while (*__tid != 0)							      \
      lll_futex_wait (__tid, *__tid, LLL_SHARED);			      \
  } while (0)

extern int __lll_timedwait_tid (int *tid, const struct timespec *abstime)
     attribute_hidden;
#define lll_timedwait_tid(tid, abstime) \
  ({									      \
    int __result = 0;							      \
    if (tid != 0)							      \
      {									      \
	if (abstime->tv_nsec < 0 || abstime->tv_nsec >= 1000000000)	      \
	  __result = EINVAL;						      \
	else								      \
	  __result = __lll_timedwait_tid (&tid, abstime);		      \
      }									      \
    __result; })

#endif  /* !__ASSEMBLER__ */

#endif  /* lowlevellock.h */
