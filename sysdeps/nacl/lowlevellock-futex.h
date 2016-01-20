/* Low-level locking access to futex facilities.  NaCl version.
   Copyright (C) 2015-2016 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef _LOWLEVELLOCK_FUTEX_H
#define _LOWLEVELLOCK_FUTEX_H   1

#include <nacl-interfaces.h>
#include <time.h>


#pragma GCC diagnostic ignored "-Wunused-value" /* XXX */

/* Values for 'private' parameter of locking macros.  Note pthreadP.h
   optimizes for these exact values, though they are not required.  */
#define LLL_PRIVATE     0
#define LLL_SHARED      128

#define FUTEX_PRIVATE_FLAG	0       /* XXX */


/* Wait while *FUTEXP == VAL for an lll_futex_wake call on FUTEXP.  */
#define lll_futex_wait(futexp, val, private) \
  ((void) (private), \
   - __nacl_irt_futex.futex_wait_abs ((volatile int *) (futexp), val, NULL))

/* Wait until a lll_futex_wake call on FUTEXP, or TIMEOUT elapses.  */
#define lll_futex_timed_wait(futexp, val, timeout, private)             \
  ({                                                                    \
    /* This timeout is relative, but the IRT call wants it absolute.  */\
    const struct timespec *_to = (timeout);                             \
    struct timespec _ts;                                                \
    int _err = 0;                                                       \
    if (_to != NULL                                                     \
	&& __glibc_likely ((_err = __nacl_irt_clock.clock_gettime	\
			    (CLOCK_REALTIME, &_ts)) == 0))		\
      {                                                                 \
	_ts.tv_sec += _to->tv_sec;                                      \
	_ts.tv_nsec += _to->tv_nsec;                                    \
	while (_ts.tv_nsec >= 1000000000)                               \
	  {                                                             \
	    _ts.tv_nsec -= 1000000000;                                  \
	    ++_ts.tv_sec;                                               \
	  }                                                             \
	_to = &_ts;                                                     \
      }                                                                 \
    if (_err == 0)                                                      \
      _err = __nacl_irt_futex.futex_wait_abs				\
	((volatile int *) (futexp), val, _to);                          \
    (void) (private);							\
    -_err;								\
  })

/* Wake up up to NR waiters on FUTEXP.  */
#define lll_futex_wake(futexp, nr, private)                     \
  ({                                                            \
    int _woken;                                                 \
    (void) (private);						\
    - __nacl_irt_futex.futex_wake ((volatile int *) (futexp), nr, &_woken); \
  })

/* NaCl does not support the requeue operation.  The only use of this is in
   pthread_cond_broadcast, which handles an error return by falling back to
   plain lll_futex_wake.  */
#define lll_futex_requeue(futexp, nr_wake, nr_move, mutex, val, private) \
  ((futexp), (nr_wake), (nr_move), (mutex), (val), (private), -ENOSYS)

/* NaCl does not support the special wake-unlock operation.  The only use
   of this is in pthread_cond_signal, which handles an error return by
   falling back to plain lll_futex_wake.  */
/* Wake up up to NR_WAKE waiters on FUTEXP and NR_WAKE2 on FUTEXP2.  */
#define lll_futex_wake_unlock(futexp, nr_wake, nr_wake2, futexp2, private) \
  ((futexp), (nr_wake), (nr_wake2), (futexp2), (private), -ENOSYS)


#endif  /* lowlevellock-futex.h */
