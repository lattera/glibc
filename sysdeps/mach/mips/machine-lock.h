/* Machine-specific definition for spin locks.  MIPS version.
   Copyright (C) 1996, 1997 Free Software Foundation, Inc.
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

#ifndef _MACHINE_LOCK_H
#define	_MACHINE_LOCK_H

/* To get the TAS pseudo-instruction. */
#include <mach/mips/mips_instruction.h>

/* The type of a spin lock variable.  */

typedef __volatile int __spin_lock_t;

/* Value to initialize `__spin_lock_t' variables to.  */

#define	__SPIN_LOCK_INITIALIZER	0


#ifndef _EXTERN_INLINE
#define _EXTERN_INLINE extern __inline
#endif

/* Unlock LOCK.  */

_EXTERN_INLINE void
__spin_unlock (__spin_lock_t *__lock)
{
  *__lock = 0;
}

/* Try to lock LOCK; return nonzero if we locked it, zero if another has.  */

_EXTERN_INLINE int
__spin_try_lock (register __spin_lock_t *__lock)
{
#if (__mips >= 2)
  int __rtn;

  __asm__ __volatile (".set noreorder");
#if (__mips64)
  __asm__ __volatile ("lld %0,0(%1)" : "=r" (__rtn) : "r" (__lock));
#else
  __asm__ __volatile ("ll %0,0(%1)" : "=r" (__rtn) : "r" (__lock));
#endif
  if (__rtn)
    return 0;
  __asm__ __volatile ("move %0,%1" : "=r" (__rtn) : "r" (__lock));
#if (__mips64)
  __asm__ __volatile ("scd %0,0(%1)" : "=r" (__rtn) : "r" (__lock));
#else
  __asm__ __volatile ("sc %0,0(%1)" : "=r" (__rtn) : "r" (__lock));
#endif
  __asm__ __volatile (".set reorder");
  return __rtn;
#else
  register int __rtn __asm__ ("a0");

  /* Use the Mach microkernel's emulated TAS pseudo-instruction.  */
  __asm__ __volatile (".set noreorder");
  __asm__ __volatile (".word %1" : "=r" (__rtn) : "i" (op_tas), "0" (__lock));
  __asm__ __volatile ("nop");
  __asm__ __volatile (".set reorder");
  return __rtn ^ (int) __lock;
#endif
}

/* Return nonzero if LOCK is locked.  */

_EXTERN_INLINE int
__spin_lock_locked (__spin_lock_t *__lock)
{
  return *__lock != 0;
}


#endif /* machine-lock.h */
