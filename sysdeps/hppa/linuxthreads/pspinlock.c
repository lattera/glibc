/* POSIX spinlock implementation.  hppa version.
   Copyright (C) 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <errno.h>
#include <pthread.h>
#include "internals.h"

int
__pthread_spin_lock (pthread_spinlock_t *lock)
{
  volatile unsigned int *addr = __ldcw_align (lock);

  while (__ldcw (addr) == 0)
    while (*addr == 0) ;

  return 0;
}
weak_alias (__pthread_spin_lock, pthread_spin_lock)


int
__pthread_spin_trylock (pthread_spinlock_t *lock)
{
  volatile unsigned int *a = __ldcw_align (lock);

  return __ldcw (a) ? 0 : EBUSY;
}
weak_alias (__pthread_spin_trylock, pthread_spin_trylock)


int
__pthread_spin_unlock (pthread_spinlock_t *lock)
{
  volatile unsigned int *a = __ldcw_align (lock);
  int tmp = 1;
  /* This should be a memory barrier to newer compilers */
  __asm__ __volatile__ ("stw,ma %1,0(%0)"
                        : : "r" (a), "r" (tmp) : "memory");           
  return 0;
}
weak_alias (__pthread_spin_unlock, pthread_spin_unlock)


int
__pthread_spin_init (pthread_spinlock_t *lock, int pshared)
{
  /* We can ignore the `pshared' parameter.  Since we are busy-waiting
     all processes which can access the memory location `lock' points
     to can use the spinlock.  */
  volatile unsigned int *a = __ldcw_align (lock);
  int tmp = 1;
  /* This should be a memory barrier to newer compilers */
  __asm__ __volatile__ ("stw,ma %1,0(%0)"
                        : : "r" (a), "r" (tmp) : "memory");           
  return 0;
}
weak_alias (__pthread_spin_init, pthread_spin_init)


int
__pthread_spin_destroy (pthread_spinlock_t *lock)
{
  /* Nothing to do.  */
  return 0;
}
weak_alias (__pthread_spin_destroy, pthread_spin_destroy)
