/* Copyright (C) 2002 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2002.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#ifndef _LOWLEVELLOCK_H
#define _LOWLEVELLOCK_H	1

#include <time.h>
#include <bits/pthreadtypes.h>

#ifndef LOCK_INSTR
# ifdef UP
#  define LOCK_INSTR	/* nothing */
# else
#  define LOCK_INSTR "lock;"
# endif
#endif

#define SYS_futex		240
#define FUTEX_WAIT		0
#define FUTEX_WAKE		1


/* Initializer for compatibility lock.  */
#define LLL_MUTEX_LOCK_INITIALIZER (0)


/* Does not preserve %eax and %ecx.  */
extern int __lll_mutex_lock_wait (int val, int *__futex)
     __attribute ((regparm (2))) attribute_hidden;
/* Does not preserver %eax, %ecx, and %edx.  */
extern int __lll_mutex_timedlock_wait (int val, int *__futex,
				 const struct timespec *abstime)
     __attribute ((regparm (3))) attribute_hidden;
/* Preserves all registers but %eax.  */
extern int __lll_mutex_unlock_wait (int *__futex)
     __attribute ((regparm (1))) attribute_hidden;


#define lll_mutex_trylock(futex) \
  ({ unsigned char ret;							      \
     __asm __volatile (LOCK_INSTR "cmpxchgl %2, %1; setne %0"		      \
		       : "=a" (ret), "=m" (futex)			      \
		       : "r" (1), "1" (futex), "0" (0)			      \
		       : "memory");					      \
     ret; })


#define lll_mutex_lock(futex) \
  (void) ({ int ignore1, ignore2;					      \
	    __asm __volatile (LOCK_INSTR "xaddl %0, %2\n\t"		      \
			      "testl %0, %0\n\t"			      \
			      "jne 1f\n\t"				      \
			      ".subsection 1\n"				      \
			      "1:\tleal %2, %%ecx\n\t"			      \
			      "call __lll_mutex_lock_wait\n\t"		      \
			      "jmp 2f\n\t"				      \
			      ".previous\n"				      \
			      "2:"					      \
			      : "=a" (ignore1), "=&c" (ignore2), "=m" (futex) \
			      : "0" (1), "2" (futex)			      \
			      : "memory"); })


#define lll_mutex_timedlock(futex, timeout) \
  ({ int result, ignore1, ignore2;					      \
     __asm __volatile (LOCK_INSTR "xaddl %0, %3\n\t"			      \
		       "testl %0, %0\n\t"				      \
		       "jne 1f\n\t"					      \
		       ".subsection 1\n"				      \
		       "1:\tleal %3, %%ecx\n\t"				      \
		       "movl %6, %%edx\n\t"				      \
		       "call __lll_mutex_timedlock_wait\n\t"		      \
		       "jmp 2f\n\t"					      \
		       ".previous\n"					      \
		       "2:"						      \
		       : "=a" (result), "=&c" (ignore1), "=&d" (ignore2),     \
			 "=m" (futex)					      \
		       : "0" (1), "3" (futex), "m" (timeout)		      \
		       : "memory");					      \
     result; })


#define lll_mutex_unlock(futex) \
  (void) ({ int ignore;							      \
            __asm __volatile (LOCK_INSTR "decl %0\n\t"			      \
			      "jne 1f\n\t"				      \
			      ".subsection 1\n"				      \
			      "1:\tleal %0, %%eax\n\t"			      \
			      "call __lll_mutex_unlock_wake\n\t"	      \
			      "jmp 2f\n\t"				      \
			      ".previous\n"				      \
			      "2:"					      \
			      : "=m" (futex), "=&a" (ignore)		      \
			      : "0" (futex)				      \
			      : "memory"); })


#define lll_mutex_islocked(futex) \
  (futex != 0)


/* We have a separate internal lock implementation which is not tied
   to binary compatibility.  */

/* Type for lock object.  */
typedef int lll_lock_t;

/* Initializers for lock.  */
#define LLL_LOCK_INITIALIZER		(1)
#define LLL_LOCK_INITIALIZER_LOCKED	(0)


extern int __lll_lock_wait (int val, int *__futex)
     __attribute ((regparm (2))) attribute_hidden;
extern int __lll_unlock_wake (int *__futex)
     __attribute ((regparm (1))) attribute_hidden;
extern int lll_unlock_wake_cb (int *__futex) attribute_hidden;


/* The states of a lock are:
    1  -  untaken
    0  -  taken by one user
   <0  -  taken by more users */


#define lll_trylock(futex) \
  ({ unsigned char ret;							      \
     __asm __volatile (LOCK_INSTR "cmpxchgl %2, %1; setne %0"		      \
		       : "=a" (ret), "=m" (futex)			      \
		       : "r" (0), "1" (futex), "0" (1)			      \
		       : "memory");					      \
     ret; })


#define lll_lock(futex) \
  (void) ({ int ignore1, ignore2;					      \
	    __asm __volatile (LOCK_INSTR "xaddl %0, %2\n\t"		      \
			      "jne 1f\n\t"				      \
			      ".subsection 1\n"				      \
			      "1:\tleal %2, %%ecx\n\t"			      \
			      "call __lll_lock_wait\n\t"		      \
			      "jmp 2f\n\t"				      \
			      ".previous\n"				      \
			      "2:"					      \
			      : "=a" (ignore1), "=&c" (ignore2), "=m" (futex) \
			      : "0" (-1), "2" (futex)			      \
			      : "memory"); })


#define lll_unlock(futex) \
  (void) ({ int ignore;							      \
            __asm __volatile (LOCK_INSTR "incl %0\n\t"			      \
			      "jng 1f\n\t"				      \
			      ".subsection 1\n"				      \
			      "1:\tleal %0, %%eax\n\t"			      \
			      "call __lll_unlock_wake\n\t"		      \
			      "jmp 2f\n\t"				      \
			      ".previous\n"				      \
			      "2:"					      \
			      : "=m" (futex), "=&a" (ignore)		      \
			      : "0" (futex)				      \
			      : "memory"); })


#define lll_islocked(futex) \
  (futex != 0)


/* The kernel notifies a process with uses CLONE_CLEARTID via futex
   wakeup when the clone terminates.  The memory location contains the
   thread ID while the clone is running and is reset to zero
   afterwards.

   The macro parameter must not have any side effect.  */
#ifdef PIC
# define LLL_TID_EBX_LOAD	"xchgl %2, %%ebx\n"
# define LLL_TID_EBX_REG	"D"
#else
# define LLL_TID_EBX_LOAD
# define LLL_TID_EBX_REG	"b"
#endif
#define lll_wait_tid(tid) \
  do {									      \
    int __ignore;							      \
    register __typeof (tid) _tid asm ("edx") = (tid);			      \
    if (_tid != 0)							      \
      __asm __volatile (LLL_TID_EBX_LOAD				      \
			"1:\tmovl %1, %%eax\n\t"			      \
			"int $0x80\n\t"					      \
			"cmpl $0, (%%ebx)\n\t"				      \
			"jne,pn 1b\n\t"					      \
			LLL_TID_EBX_LOAD				      \
			: "=&a" (__ignore)				      \
			: "i" (SYS_futex), LLL_TID_EBX_REG (&tid), "S" (0),   \
			  "c" (FUTEX_WAIT), "d" (_tid));		      \
  } while (0)

extern int __lll_timedwait_tid (int *tid, const struct timespec *abstime)
     __attribute__ ((regparm (2))) attribute_hidden;
#define lll_timedwait_tid(tid, abstime) \
  ({									      \
    int __result = 0;							      \
    if (tid != 0)							      \
      {									      \
	if (abstime == NULL || abstime->tv_nsec >= 1000000000)		      \
	  __result = EINVAL;						      \
	else								      \
	  __result = __lll_timedwait_tid (&tid, abstime);		      \
      }									      \
    __result; })


#define lll_wake_tid(tid) \
  do {									      \
    int __ignore;							      \
    (tid) = 0;								      \
    __asm __volatile (LLL_TID_EBX_LOAD					      \
		      "\tint $0x80\n\t"					      \
		      LLL_TID_EBX_LOAD					      \
		      : "=a" (__ignore)					      \
		      : "0" (SYS_futex), LLL_TID_EBX_REG (&(tid)), "S" (0),   \
			"c" (FUTEX_WAKE), "d" (0x7fffffff));		      \
  } while (0)


/* Conditional variable handling.  */

extern void __lll_cond_wait (pthread_cond_t *cond)
     __attribute ((regparm (1))) attribute_hidden;
extern int __lll_cond_timedwait (pthread_cond_t *cond,
				 const struct timespec *abstime)
     __attribute ((regparm (2))) attribute_hidden;
extern void __lll_cond_wake (pthread_cond_t *cond)
     __attribute ((regparm (1))) attribute_hidden;
extern void __lll_cond_broadcast (pthread_cond_t *cond)
     __attribute ((regparm (1))) attribute_hidden;


#define lll_cond_wait(cond) \
  __lll_cond_wait (cond)
#define lll_cond_timedwait(cond, abstime) \
  __lll_cond_timedwait (cond, abstime)
#define lll_cond_wake(cond) \
  __lll_cond_wake (cond)
#define lll_cond_broadcast(cond) \
  __lll_cond_broadcast (cond)


#endif	/* lowlevellock.h */
