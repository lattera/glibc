/* elision-unlock.c: Commit an elided pthread lock.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include "pthreadP.h"
#include "lowlevellock.h"
#include "htm.h"

int
__lll_unlock_elision (int *lock, short *adapt_count, int pshared)
{
  /* When the lock was free we're in a transaction.  */
  if (*lock == 0)
    __libc_tend (0);
  else
    {
      lll_unlock ((*lock), pshared);

      /* Update the adapt count AFTER completing the critical section.
         Doing this here prevents unneeded stalling when entering
         a critical section.  Saving about 8% runtime on P8.  */
      if (*adapt_count > 0)
	(*adapt_count)--;
    }
  return 0;
}
