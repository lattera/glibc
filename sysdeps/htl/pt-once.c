/* pthread_once.  Generic version.
   Copyright (C) 2002-2018 Free Software Foundation, Inc.
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
   License along with the GNU C Library;  if not, see
   <http://www.gnu.org/licenses/>.  */

#include <pthread.h>
#include <atomic.h>

#include <pt-internal.h>

int
__pthread_once (pthread_once_t *once_control, void (*init_routine) (void))
{
  atomic_full_barrier ();
  if (once_control->__run == 0)
    {
      __pthread_spin_lock (&once_control->__lock);

      if (once_control->__run == 0)
	{
	  init_routine ();
	  atomic_full_barrier ();
	  once_control->__run = 1;
	}

      __pthread_spin_unlock (&once_control->__lock);
    }

  return 0;
}
strong_alias (__pthread_once, pthread_once);
