/* Machine-specific definition for spin locks.  MIPS version.
Copyright (C) 1994 Free Software Foundation, Inc.
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
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#ifndef _MACHINE_LOCK_H
#define	_MACHINE_LOCK_H

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
  register int __rtn;
  __asm__ __volatile (".set noreorder");
#if 0
  __asm__ __volatile ("lw %0,0(%1)": "=r" (__rtn) : "r" (__lock));
  __asm__ __volatile ("sw %0,0(%0)": : "r" (__lock));
  __asm__ __volatile ("xor %0,%1,%0": "=r" (__rtn) : "r" (__lock));
#else
  /* Use the Mach microkernel's emulated TAS pseudo-instruction.  */
  register int __rtn __asm__ ("a0");
  __asm__ __volatile (".word 0xf ! %0 " : "=r" (__rtn) : "0" (__lock));
#endif
  __asm__ __volatile (".set reorder");
  return __rtn ^ (int) __lock;
}

/* Return nonzero if LOCK is locked.  */

_EXTERN_INLINE int
__spin_lock_locked (__spin_lock_t *__lock)
{
  return *__lock != 0;
}


#endif /* machine-lock.h */
