/* POSIX spinlock implementation.  MIPS version.
   Copyright (C) 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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


/* This implementation is similar to the one used in the Linux kernel.  */
int
__pthread_spin_lock (pthread_spinlock_t *lock)
{
  unsigned int tmp;

  asm volatile
    (".set\tnoreorder\t\t\t# spin_lock\n"
     ".set\tpush\n"
     ".set\tmips2\n"
     "1:\tll\t%1, %2\n\t"
     "bnez\t%1, 1b\n\t"
     " li\t%1, 1\n\t"
     "sc\t%1, %0\n\t"
     "beqz\t%1, 1b\n\t"
     ".set\tpop\n"
     ".set\treorder"
     : "=o" (*lock), "=&r" (tmp)
     : "o" (*lock)
     : "memory");

  return 0;
}
weak_alias (__pthread_spin_lock, pthread_spin_lock)


int
__pthread_spin_trylock (pthread_spinlock_t *lock)
{
  /* To be done.  */
  return 0;
}
weak_alias (__pthread_spin_trylock, pthread_spin_trylock)


int
__pthread_spin_unlock (pthread_spinlock_t *lock)
{
  asm volatile
    (".set\tnoreorder\t\t\t# spin_unlock\n\t"
     "sw\t$0, %0\n\t"
     ".set\treorder" 
     : "=o" (*lock)
     : "o" (*lock)
     : "memory");
  return 0;
}
weak_alias (__pthread_spin_unlock, pthread_spin_unlock)


int
__pthread_spin_init (pthread_spinlock_t *lock, int pshared)
{
  /* We can ignore the `pshared' parameter.  Since we are busy-waiting
     all processes which can access the memory location `lock' points
     to can use the spinlock.  */
  *lock = 1;
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
