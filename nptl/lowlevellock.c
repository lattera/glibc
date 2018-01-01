/* low level locking for pthread library.  Generic futex-using version.
   Copyright (C) 2003-2018 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <sysdep.h>
#include <lowlevellock.h>
#include <sys/time.h>
#include <atomic.h>

void
__lll_lock_wait_private (int *futex)
{
  if (*futex == 2)
    lll_futex_wait (futex, 2, LLL_PRIVATE); /* Wait if *futex == 2.  */

  while (atomic_exchange_acq (futex, 2) != 0)
    lll_futex_wait (futex, 2, LLL_PRIVATE); /* Wait if *futex == 2.  */
}


/* This function doesn't get included in libc.  */
#if IS_IN (libpthread)
void
__lll_lock_wait (int *futex, int private)
{
  if (*futex == 2)
    lll_futex_wait (futex, 2, private); /* Wait if *futex == 2.  */

  while (atomic_exchange_acq (futex, 2) != 0)
    lll_futex_wait (futex, 2, private); /* Wait if *futex == 2.  */
}
#endif
