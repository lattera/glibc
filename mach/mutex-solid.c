/* Stub versions of mutex_lock_solid/mutex_unlock_solid for no -lthreads.
   Copyright (C) 1995, 1997 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <lock-intern.h>
#include <cthreads.h>

/* If cthreads is linked in, it will define these functions itself to do
   real cthreads mutex locks.  This file will only be linked in when
   cthreads is not used, and `mutexes' are in fact just spin locks (and
   some unused storage).  */

void
__mutex_lock_solid (void *lock)
{
  __spin_lock_solid (lock);
}

void
__mutex_unlock_solid (void *lock)
{
}
