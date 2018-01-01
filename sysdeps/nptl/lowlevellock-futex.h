/* Low-level locking access to futex facilities.  Stub version.
   Copyright (C) 2014-2018 Free Software Foundation, Inc.
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

#include <errno.h>


/* Values for 'private' parameter of locking macros.  Note pthreadP.h
   optimizes for these exact values, though they are not required.  */
#define LLL_PRIVATE     0
#define LLL_SHARED      128


/* For most of these macros, the return value is never really used.
   Nevertheless, the protocol is that each one returns a negated errno
   code for failure or zero for success.  (Note that the corresponding
   Linux system calls can sometimes return positive values for success
   cases too.  We never use those values.)  */


/* Wait while *FUTEXP == VAL for an lll_futex_wake call on FUTEXP.  */
#define lll_futex_wait(futexp, val, private) \
  lll_futex_timed_wait (futexp, val, NULL, private)

/* Wait until a lll_futex_wake call on FUTEXP, or TIMEOUT elapses.  */
#define lll_futex_timed_wait(futexp, val, timeout, private)             \
  -ENOSYS

/* This macro should be defined only if FUTEX_CLOCK_REALTIME is also defined.
   If CLOCKBIT is zero, this is identical to lll_futex_timed_wait.
   If CLOCKBIT has FUTEX_CLOCK_REALTIME set, then it's the same but
   TIMEOUT is counted by CLOCK_REALTIME rather than CLOCK_MONOTONIC.  */
#define lll_futex_timed_wait_bitset(futexp, val, timeout, clockbit, private) \
  -ENOSYS

/* Wake up up to NR waiters on FUTEXP.  */
#define lll_futex_wake(futexp, nr, private)                             \
  -ENOSYS

/* Wake up up to NR_WAKE waiters on FUTEXP.  Move up to NR_MOVE of the
   rest from waiting on FUTEXP to waiting on MUTEX (a different futex).  */
#define lll_futex_requeue(futexp, nr_wake, nr_move, mutex, val, private) \
  -ENOSYS

/* Wake up up to NR_WAKE waiters on FUTEXP and NR_WAKE2 on FUTEXP2.  */
#define lll_futex_wake_unlock(futexp, nr_wake, nr_wake2, futexp2, private) \
  -ENOSYS


/* Like lll_futex_wait (FUTEXP, VAL, PRIVATE) but with the expectation
   that lll_futex_cmp_requeue_pi (FUTEXP, _, _, MUTEX, _, PRIVATE) will
   be used to do the wakeup.  Confers priority-inheritance behavior on
   the waiter.  */
#define lll_futex_wait_requeue_pi(futexp, val, mutex, private) \
  lll_futex_timed_wait_requeue_pi (futexp, val, NULL, 0, mutex, private)

/* Like lll_futex_wait_requeue_pi, but with a timeout.  */
#define lll_futex_timed_wait_requeue_pi(futexp, val, timeout, clockbit, \
                                        mutex, private)                 \
  -ENOSYS

/* Like lll_futex_requeue, but pairs with lll_futex_wait_requeue_pi
   and inherits priority from the waiter.  */
#define lll_futex_cmp_requeue_pi(futexp, nr_wake, nr_move, mutex,       \
                                 val, private)                          \
  -ENOSYS


#endif  /* lowlevellock-futex.h */
