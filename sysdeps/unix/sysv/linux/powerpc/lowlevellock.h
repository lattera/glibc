/* PowerPC specific lock definitions.
   Copyright (C) 2015-2018 Free Software Foundation, Inc.
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

#ifndef _POWERPC_LOWLEVELLOCK_H
#define _POWERPC_LOWLEVELLOCK_H 1

#include <sysdeps/nptl/lowlevellock.h>

/* Transactional lock elision definitions.  */
extern int __lll_timedlock_elision
  (int *futex, short *adapt_count, const struct timespec *timeout, int private)
  attribute_hidden;

#define lll_timedlock_elision(futex, adapt_count, timeout, private)	      \
  __lll_timedlock_elision(&(futex), &(adapt_count), timeout, private)

extern int __lll_lock_elision (int *futex, short *adapt_count, int private)
  attribute_hidden;

extern int __lll_unlock_elision (int *lock, short *adapt_count, int private)
  attribute_hidden;

extern int __lll_trylock_elision(int *lock, short *adapt_count)
  attribute_hidden;

#define lll_lock_elision(futex, adapt_count, private) \
  __lll_lock_elision (&(futex), &(adapt_count), private)
#define lll_unlock_elision(futex, adapt_count, private) \
  __lll_unlock_elision (&(futex), &(adapt_count), private)
#define lll_trylock_elision(futex, adapt_count) \
  __lll_trylock_elision (&(futex), &(adapt_count))

#endif
