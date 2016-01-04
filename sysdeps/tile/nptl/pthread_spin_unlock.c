/* pthread_spin_unlock -- unlock a spin lock.  Tile version.
   Copyright (C) 2012-2016 Free Software Foundation, Inc.
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

#include "pthreadP.h"
#include <atomic.h>

int
pthread_spin_unlock (pthread_spinlock_t *lock)
{
#ifdef __tilegx__
  /* Use exchange() to bypass the write buffer. */
  atomic_exchange_rel (lock, 0);
#else
  atomic_full_barrier ();
  *lock = 0;
#endif
  return 0;
}
