/* Copyright (C) 2002, 2003, 2004 Free Software Foundation, Inc.
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
#include <sys/param.h>
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
#define LLL_MUTEX_LOCK_INITIALIZER		(0)
#define LLL_MUTEX_LOCK_INITIALIZER_LOCKED	(1)
#define LLL_MUTEX_LOCK_INITIALIZER_WAITERS	(2)


#ifdef PIC
# define LLL_EBX_LOAD	"xchgl %2, %%ebx\n"
# define LLL_EBX_REG	"D"
#else
# define LLL_EBX_LOAD
# define LLL_EBX_REG	"b"
#endif

#ifdef I386_USE_SYSENTER
# ifdef SHARED
#  define LLL_ENTER_KERNEL	"call *%%gs:%P6\n\t"
# else
#  define LLL_ENTER_KERNEL	"call *_dl_sysinfo\n\t"
# endif
#else
# define LLL_ENTER_KERNEL	"int $0x80\n\t"
#endif

/* Delay in spinlock loop.  */
#define BUSY_WAIT_NOP          asm ("rep; nop")


#define lll_futex_wait(futex, val) \
  do {									      \
    int __ignore;							      \
    register __typeof (val) _val asm ("edx") = (val);			      \
    __asm __volatile (LLL_EBX_LOAD					      \
		      LLL_ENTER_KERNEL					      \
		      LLL_EBX_LOAD					      \
		      : "=a" (__ignore)					      \
		      : "0" (SYS_futex), LLL_EBX_REG (futex), "S" (0),	      \
			"c" (FUTEX_WAIT), "d" (_val),			      \
			"i" (offsetof (tcbhead_t, sysinfo)));		      \
  } while (0)


#define lll_futex_wake(futex, nr) \
  do {									      \
    int __ignore;							      \
    register __typeof (nr) _nr asm ("edx") = (nr);			      \
    __asm __volatile (LLL_EBX_LOAD					      \
		      LLL_ENTER_KERNEL					      \
		      LLL_EBX_LOAD					      \
		      : "=a" (__ignore)					      \
		      : "0" (SYS_futex), LLL_EBX_REG (futex),		      \
			"c" (FUTEX_WAKE), "d" (_nr),			      \
			"i" (0) /* phony, to align next arg's number */,      \
			"i" (offsetof (tcbhead_t, sysinfo)));		      \
  } while (0)


/* Does not preserve %eax and %ecx.  */
extern int __lll_mutex_lock_wait (int val, int *__futex)
     __attribute ((regparm (2))) attribute_hidden;
/* Does not preserve %eax, %ecx, and %edx.  */
extern int __lll_mutex_timedlock_wait (int val, int *__futex,
				       const struct timespec *abstime)
     __attribute ((regparm (3))) attribute_hidden;
/* Preserves all registers but %eax.  */
extern int __lll_mutex_unlock_wake (int *__futex)
     __attribute ((regparm (1))) attribute_hidden;


/* NB: in the lll_mutex_trylock macro we simply return the value in %eax
   after the cmpxchg instruction.  In case the operation succeded this
   value is zero.  In case the operation failed, the cmpxchg instruction
   has loaded the current value of the memory work which is guaranteed
   to be nonzero.  */
#define lll_mutex_trylock(futex) \
  ({ int ret;								      \
     __asm __volatile (LOCK_INSTR "cmpxchgl %2, %1"			      \
		       : "=a" (ret), "=m" (futex)			      \
		       : "r" (LLL_MUTEX_LOCK_INITIALIZER_LOCKED), "m" (futex),\
			 "0" (LLL_MUTEX_LOCK_INITIALIZER)		      \
		       : "memory");					      \
     ret; })


#define lll_mutex_cond_trylock(futex) \
  ({ int ret;								      \
     __asm __volatile (LOCK_INSTR "cmpxchgl %2, %1"			      \
		       : "=a" (ret), "=m" (futex)			      \
		       : "r" (LLL_MUTEX_LOCK_INITIALIZER_WAITERS),	      \
			  "m" (futex), "0" (LLL_MUTEX_LOCK_INITIALIZER)	      \
		       : "memory");					      \
     ret; })


#define lll_mutex_lock(futex) \
  (void) ({ int ignore1, ignore2;					      \
	    __asm __volatile (LOCK_INSTR "cmpxchgl %1, %2\n\t"		      \
			      "jnz _L_mutex_lock_%=\n\t"		      \
			      ".subsection 1\n\t"			      \
			      ".type _L_mutex_lock_%=,@function\n"	      \
			      "_L_mutex_lock_%=:\n\t"			      \
			      "leal %2, %%ecx\n\t"			      \
			      "call __lll_mutex_lock_wait\n\t"		      \
			      "jmp 1f\n\t"				      \
			      ".size _L_mutex_lock_%=,.-_L_mutex_lock_%=\n"   \
			      ".previous\n"				      \
			      "1:"					      \
			      : "=a" (ignore1), "=c" (ignore2), "=m" (futex)  \
			      : "0" (0), "1" (1), "m" (futex)		      \
			      : "memory"); })


/* Special version of lll_mutex_lock which causes the unlock function to
   always wakeup waiters.  */
#define lll_mutex_cond_lock(futex) \
  (void) ({ int ignore1, ignore2;					      \
	    __asm __volatile (LOCK_INSTR "cmpxchgl %1, %2\n\t"		      \
			      "jnz _L_mutex_cond_lock_%=\n\t"		      \
			      ".subsection 1\n\t"			      \
			      ".type _L_mutex_cond_lock_%=,@function\n"	      \
			      "_L_mutex_cond_lock_%=:\n\t"		      \
			      "leal %2, %%ecx\n\t"			      \
			      "call __lll_mutex_lock_wait\n\t"		      \
			      "jmp 1f\n\t"				      \
			      ".size _L_mutex_cond_lock_%=,.-_L_mutex_cond_lock_%=\n"   \
			      ".previous\n"				      \
			      "1:"					      \
			      : "=a" (ignore1), "=c" (ignore2), "=m" (futex)  \
			      : "0" (0), "1" (2), "m" (futex)		      \
			      : "memory"); })


#define lll_mutex_timedlock(futex, timeout) \
  ({ int result, ignore1, ignore2;					      \
     __asm __volatile (LOCK_INSTR "cmpxchgl %1, %3\n\t"			      \
		       "jnz _L_mutex_timedlock_%=\n\t"			      \
		       ".subsection 1\n\t"				      \
		       ".type _L_mutex_timedlock_%=,@function\n"	      \
		       "_L_mutex_timedlock_%=:\n\t"			      \
		       "leal %3, %%ecx\n\t"				      \
		       "movl %7, %%edx\n\t"				      \
		       "call __lll_mutex_timedlock_wait\n\t"		      \
		       "jmp 1f\n\t"					      \
		       ".size _L_mutex_timedlock_%=,.-_L_mutex_timedlock_%=\n"\
		       ".previous\n"					      \
		       "1:"						      \
		       : "=a" (result), "=c" (ignore1), "=&d" (ignore2),      \
			 "=m" (futex)					      \
		       : "0" (0), "1" (1), "m" (futex), "m" (timeout)	      \
		       : "memory");					      \
     result; })


#define lll_mutex_unlock(futex) \
  (void) ({ int ignore;							      \
            __asm __volatile (LOCK_INSTR "subl $1,%0\n\t"		      \
			      "jne _L_mutex_unlock_%=\n\t"		      \
			      ".subsection 1\n\t"			      \
			      ".type _L_mutex_unlock_%=,@function\n"	      \
			      "_L_mutex_unlock_%=:\n\t"			      \
			      "leal %0, %%eax\n\t"			      \
			      "call __lll_mutex_unlock_wake\n\t"	      \
			      "jmp 1f\n\t"				      \
			      ".size _L_mutex_unlock_%=,.-_L_mutex_unlock_%=\n" \
			      ".previous\n"				      \
			      "1:"					      \
			      : "=m" (futex), "=&a" (ignore)		      \
			      : "m" (futex)				      \
			      : "memory"); })


#define lll_mutex_islocked(futex) \
  (futex != 0)


/* We have a separate internal lock implementation which is not tied
   to binary compatibility.  */

/* Type for lock object.  */
typedef int lll_lock_t;

/* Initializers for lock.  */
#define LLL_LOCK_INITIALIZER		(0)
#define LLL_LOCK_INITIALIZER_LOCKED	(1)


extern int __lll_lock_wait (int val, int *__futex)
     __attribute ((regparm (2))) attribute_hidden;
extern int __lll_unlock_wake (int *__futex)
     __attribute ((regparm (1))) attribute_hidden;
extern int lll_unlock_wake_cb (int *__futex) attribute_hidden;


/* The states of a lock are:
    0  -  untaken
    1  -  taken by one user
    2  -  taken by more users */


#if defined NOT_IN_libc || defined UP
# define lll_trylock(futex) lll_mutex_trylock (futex)
# define lll_lock(futex) lll_mutex_lock (futex)
# define lll_unlock(futex) lll_mutex_unlock (futex)
#else
/* Special versions of the macros for use in libc itself.  They avoid
   the lock prefix when the thread library is not used.

   XXX In future we might even want to avoid it on UP machines.  */
# include <tls.h>

# define lll_trylock(futex) \
  ({ unsigned char ret;							      \
     __asm __volatile ("cmpl $0, %%gs:%P5\n\t"				      \
		       "je,pt 0f\n\t"					      \
		       "lock\n"						      \
		       "0:\tcmpxchgl %2, %1; setne %0"			      \
		       : "=a" (ret), "=m" (futex)			      \
		       : "r" (LLL_MUTEX_LOCK_INITIALIZER_LOCKED), "m" (futex),\
			 "0" (LLL_MUTEX_LOCK_INITIALIZER),		      \
		         "i" (offsetof (tcbhead_t, multiple_threads))	      \
		       : "memory");					      \
     ret; })


# define lll_lock(futex) \
  (void) ({ int ignore1, ignore2;					      \
	    __asm __volatile ("cmpl $0, %%gs:%P6\n\t"			      \
			      "je,pt 0f\n\t"				      \
			      "lock\n"					      \
			      "0:\tcmpxchgl %1, %2\n\t"			      \
			      "jnz _L_mutex_lock_%=\n\t"		      \
			      ".subsection 1\n\t"			      \
			      ".type _L_mutex_lock_%=,@function\n"	      \
			      "_L_mutex_lock_%=:\n\t"			      \
			      "leal %2, %%ecx\n\t"			      \
			      "call __lll_mutex_lock_wait\n\t"		      \
			      "jmp 1f\n\t"				      \
			      ".size _L_mutex_lock_%=,.-_L_mutex_lock_%=\n"   \
			      ".previous\n"				      \
			      "1:"					      \
			      : "=a" (ignore1), "=c" (ignore2), "=m" (futex)  \
			      : "0" (0), "1" (1), "m" (futex),		      \
		                "i" (offsetof (tcbhead_t, multiple_threads))  \
			      : "memory"); })


# define lll_unlock(futex) \
  (void) ({ int ignore;							      \
            __asm __volatile ("cmpl $0, %%gs:%P3\n\t"			      \
			      "je,pt 0f\n\t"				      \
			      "lock\n"					      \
			      "0:\tsubl $1,%0\n\t"		      \
			      "jne _L_mutex_unlock_%=\n\t"		      \
			      ".subsection 1\n\t"			      \
			      ".type _L_mutex_unlock_%=,@function\n"	      \
			      "_L_mutex_unlock_%=:\n\t"			      \
			      "leal %0, %%eax\n\t"			      \
			      "call __lll_mutex_unlock_wake\n\t"	      \
			      "jmp 1f\n\t"				      \
			      ".size _L_mutex_unlock_%=,.-_L_mutex_unlock_%=\n" \
			      ".previous\n"				      \
			      "1:"					      \
			      : "=m" (futex), "=&a" (ignore)		      \
			      : "m" (futex),				      \
				"i" (offsetof (tcbhead_t, multiple_threads))  \
			      : "memory"); })
#endif


#define lll_islocked(futex) \
  (futex != LLL_LOCK_INITIALIZER)


/* The kernel notifies a process with uses CLONE_CLEARTID via futex
   wakeup when the clone terminates.  The memory location contains the
   thread ID while the clone is running and is reset to zero
   afterwards.

   The macro parameter must not have any side effect.  */
#define lll_wait_tid(tid) \
  do {									      \
    int __ignore;							      \
    register __typeof (tid) _tid asm ("edx") = (tid);			      \
    if (_tid != 0)							      \
      __asm __volatile (LLL_EBX_LOAD					      \
			"1:\tmovl %1, %%eax\n\t"			      \
			LLL_ENTER_KERNEL				      \
			"cmpl $0, (%%ebx)\n\t"				      \
			"jne,pn 1b\n\t"					      \
			LLL_EBX_LOAD					      \
			: "=&a" (__ignore)				      \
			: "i" (SYS_futex), LLL_EBX_REG (&tid), "S" (0),	      \
			  "c" (FUTEX_WAIT), "d" (_tid),			      \
			  "i" (offsetof (tcbhead_t, sysinfo)));		      \
  } while (0)

extern int __lll_timedwait_tid (int *tid, const struct timespec *abstime)
     __attribute__ ((regparm (2))) attribute_hidden;
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
