/* Commit an elided pthread lock.
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

#include <pthreadP.h>
#include <lowlevellock.h>

int
__lll_unlock_elision(int *futex, int private)
{
  /* If the lock is free, we elided the lock earlier.  This does not
     necessarily mean that we are in a transaction, because the user code may
     have closed the transaction, but that is impossible to detect reliably.  */
  if (*futex == 0)
    {
      __asm__ volatile (".machinemode \"zarch_nohighgprs\"\n\t"
			".machine \"all\""
			: : : "memory");
      __builtin_tend();
    }
  else
    lll_unlock ((*futex), private);
  return 0;
}
