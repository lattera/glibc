/* Low-level lock implementation.  NaCl version.
   Copyright (C) 2015-2016 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef _LOWLEVELLOCK_H

/* Everything except the exit handling is the same as the generic code.  */
# include <sysdeps/nptl/lowlevellock.h>

/* See exit-thread.h for details.  */
# define NACL_EXITING_TID	1

# undef lll_wait_tid
# define lll_wait_tid(tid)				\
  do {							\
    __typeof (tid) __tid;				\
    volatile __typeof (tid) *__tidp = &(tid);		\
    while ((__tid = atomic_load_relaxed (__tidp)) != 0) \
      {							\
	if (__tid == NACL_EXITING_TID)			\
	  atomic_spin_nop ();				\
	else						\
	  lll_futex_wait (__tidp, __tid, LLL_PRIVATE);	\
      }							\
  } while (0)

#endif	/* lowlevellock.h */
