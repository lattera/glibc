/* POSIX spinlock implementation.  ia64 version.
   Copyright (C) 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Jes Sorensen <jes@linuxcare.com>

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <errno.h>
#include <pthread.h>
#include "internals.h"

/* This implementation is inspired by the implementation used in the
   Linux kernel. */

int
__pthread_spin_lock (pthread_spinlock_t *lock)
{
  asm volatile
    ("mov ar.ccv = r0\n\t"
     "mov r3 = 1\n\t"
     ";;\n"
     "1:\n\t"
     "ld4 r2 = %0\n\t"
     ";;\n\t"
     "cmp4.eq p0, p7 = r0, r2\n\t"
     "(p7) br.cond.spnt.few 1b \n\t"
     "cmpxchg4.acq r2 = %0, r3, ar.ccv\n\t"
     ";;\n\t"
     "cmp4.eq p0, p7 = r0, r2\n\t"
     "(p7) br.cond.spnt.few 1b\n\t"
     ";;\n"
     :: "m" (lock) : "r2", "r3", "p7", "memory");
  return 0;
}
weak_alias (__pthread_spin_lock, pthread_spin_lock)


int
__pthread_spin_trylock (pthread_spinlock_t *lock)
{
  int oldval;

  asm volatile
    ("mov ar.ccv = r0\n\t"
     "mov r2 = 1\n\t"
     ";;\n\t"
     "cmpxchg4.acq %0 = %1, r2, ar.ccv\n\t"
     ";;\n"
     : "=r" (oldval) : "m" (lock) : "r2", "memory");
  return oldval > 0 ? 0 : EBUSY;
}
weak_alias (__pthread_spin_trylock, pthread_spin_trylock)


int
__pthread_spin_unlock (pthread_spinlock_t *lock)
{
  return *lock = 0;
}
weak_alias (__pthread_spin_unlock, pthread_spin_unlock)


int
__pthread_spin_init (pthread_spinlock_t *lock, int pshared)
{
  /* We can ignore the `pshared' parameter.  Since we are busy-waiting
     all processes which can access the memory location `lock' points
     to can use the spinlock.  */
  return *lock = 0;
}
weak_alias (__pthread_spin_init, pthread_spin_init)


int
__pthread_spin_destroy (pthread_spinlock_t *lock)
{
  /* Nothing to do.  */
  return 0;
}
weak_alias (__pthread_spin_destroy, pthread_spin_destroy)
