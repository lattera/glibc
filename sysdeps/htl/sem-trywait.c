/* Lock a semaphore if it does not require blocking.  Generic version.
   Copyright (C) 2005-2018 Free Software Foundation, Inc.
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

#include <semaphore.h>
#include <errno.h>

#include <pt-internal.h>

int
__sem_trywait (sem_t *sem)
{
  __pthread_spin_lock (&sem->__lock);
  if (sem->__value > 0)
    /* Successful down.  */
    {
      sem->__value--;
      __pthread_spin_unlock (&sem->__lock);
      return 0;
    }
  __pthread_spin_unlock (&sem->__lock);

  errno = EAGAIN;
  return -1;
}

strong_alias (__sem_trywait, sem_trywait);
