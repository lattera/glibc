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
#include <sgidefs.h>
#include <sys/tas.h>


#if (_MIPS_ISA >= _MIPS_ISA_MIPS2)

/* This implementation is similar to the one used in the Linux kernel.  */
int
__pthread_spin_lock (pthread_spinlock_t *lock)
{
  unsigned int tmp;

  asm volatile
    ("\t\t\t# spin_lock\n\t"
     "1:\n\t"
     "ll	%1,%2\n\t"
     ".set	push\n\t"
     ".set	noreorder\n\t"
     "bnez	%1,1b\n\t"
     " li	%1,1\n\t"
     ".set	pop\n\t"
     "sc	%1,%0\n\t"
     "beqz	%1,1b"
     : "=m" (*lock), "=&r" (tmp)
     : "m" (*lock)
     : "memory");

  return 0;
}

#else /* !(_MIPS_ISA >= _MIPS_ISA_MIPS2) */

int
__pthread_spin_lock (pthread_spinlock_t *lock)
{
  while (_test_and_set (lock, 1));
  return 0;
}

#endif /* !(_MIPS_ISA >= _MIPS_ISA_MIPS2) */

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
    ("\t\t\t# spin_unlock\n\t"
     "sw	$0,%0"
     : "=m" (*lock)
     :
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
