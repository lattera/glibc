/* Low-level locking access to futex facilities.  Linux/i386 version.
   Copyright (C) 2014-2015 Free Software Foundation, Inc.
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

#ifndef _LOWLEVELLOCK_FUTEX_H
#define _LOWLEVELLOCK_FUTEX_H	1

#define FUTEX_WAIT		0
#define FUTEX_WAKE		1
#define FUTEX_CMP_REQUEUE	4
#define FUTEX_WAKE_OP		5
#define FUTEX_LOCK_PI		6
#define FUTEX_UNLOCK_PI		7
#define FUTEX_TRYLOCK_PI	8
#define FUTEX_WAIT_BITSET	9
#define FUTEX_WAKE_BITSET	10
#define FUTEX_WAIT_REQUEUE_PI	11
#define FUTEX_CMP_REQUEUE_PI	12
#define FUTEX_PRIVATE_FLAG	128
#define FUTEX_CLOCK_REALTIME	256

#define FUTEX_BITSET_MATCH_ANY	0xffffffff

#define FUTEX_OP_CLEAR_WAKE_IF_GT_ONE	((4 << 24) | 1)

/* Values for 'private' parameter of locking macros.  Yes, the
   definition seems to be backwards.  But it is not.  The bit will be
   reversed before passing to the system call.  */
#define LLL_PRIVATE	0
#define LLL_SHARED	FUTEX_PRIVATE_FLAG


#if IS_IN (libc) || IS_IN (rtld)
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
   : ({ unsigned int __fl = ((private) ^ FUTEX_PRIVATE_FLAG);		      \
	asm ("andl %%gs:%P1, %0" : "+r" (__fl)				      \
	     : "i" (offsetof (struct pthread, header.private_futex)));	      \
	__fl | (fl); }))
# endif
#endif


#ifndef __ASSEMBLER__

/* To avoid naming conflicts with lowlevellock.h, use a different prefix
   here.  */
#ifdef PIC
# define LLLF_EBX_LOAD	"xchgl %2, %%ebx\n"
# define LLLF_EBX_REG	"D"
#else
# define LLLF_EBX_LOAD
# define LLLF_EBX_REG	"b"
#endif

#ifdef I386_USE_SYSENTER
# ifdef SHARED
#  define LLLF_ENTER_KERNEL	"call *%%gs:%P6\n\t"
# else
#  define LLLF_ENTER_KERNEL	"call *_dl_sysinfo\n\t"
# endif
#else
# define LLLF_ENTER_KERNEL	"int $0x80\n\t"
#endif


#define lll_futex_wait(futex, val, private) \
  lll_futex_timed_wait (futex, val, NULL, private)


#define lll_futex_timed_wait(futex, val, timeout, private) \
  ({									      \
    int __status;							      \
    register __typeof (val) _val asm ("edx") = (val);			      \
    __asm __volatile (LLLF_EBX_LOAD					      \
		      LLLF_ENTER_KERNEL					      \
		      LLLF_EBX_LOAD					      \
		      : "=a" (__status)					      \
		      : "0" (SYS_futex), LLLF_EBX_REG (futex), "S" (timeout),  \
			"c" (__lll_private_flag (FUTEX_WAIT, private)),	      \
			"d" (_val), "i" (offsetof (tcbhead_t, sysinfo))	      \
		      : "memory");					      \
    __status;								      \
  })


#define lll_futex_wake(futex, nr, private) \
  ({									      \
    int __status;							      \
    register __typeof (nr) _nr asm ("edx") = (nr);			      \
    LIBC_PROBE (lll_futex_wake, 3, futex, nr, private);                       \
    __asm __volatile (LLLF_EBX_LOAD					      \
		      LLLF_ENTER_KERNEL					      \
		      LLLF_EBX_LOAD					      \
		      : "=a" (__status)					      \
		      : "0" (SYS_futex), LLLF_EBX_REG (futex),		      \
			"c" (__lll_private_flag (FUTEX_WAKE, private)),	      \
			"d" (_nr),					      \
			"i" (0) /* phony, to align next arg's number */,      \
			"i" (offsetof (tcbhead_t, sysinfo)));		      \
    __status;								      \
  })


#endif  /* !__ASSEMBLER__ */

#endif	/* lowlevellock-futex.h */
