/* Machine-specific definition for spin locks.  Alpha version.
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

typedef __volatile long int __spin_lock_t;

/* Value to initialize `__spin_lock_t' variables to.  */

#define	__SPIN_LOCK_INITIALIZER	0L


#ifndef _EXTERN_INLINE
#define _EXTERN_INLINE extern __inline
#endif

/* Unlock LOCK.  */

_EXTERN_INLINE void
__spin_unlock (__spin_lock_t *__lock)
{
  __asm__ __volatile__ (".set noreorder\n"
			"mb; stq $31, %0; mb\n"
			".set reorder"
			: "=m" (__lock));
}

/* Try to lock LOCK; return nonzero if we locked it, zero if another has.  */

_EXTERN_INLINE int
__spin_try_lock (register __spin_lock_t *__lock)
{
  register long int __rtn, __tmp;

  do
    {
      __asm__ __volatile__ (".set noreorder\n"
			    /* %0 is TMP, %1 is RTN, %2 is LOCK.  */
			    "mb; ldq_l %0,%2\n" /* Load lock into TMP.  */
			    "or $31,2,%1\n" /* Locked value in RTN.  */
			    ".set reorder"
			    : "=r" (__tmp), "=r" (__rtn) : "m" (__lock));
      if (__tmp)
	/* The lock is already taken.  */
	return 0;

      /* The lock is not taken; try to get it now.  */
      __asm__ __volatile__ ("stq_c %0,%1" : "+r" (__rtn), "+m" (__lock));
      /* RTN is clear if stq_c was interrupted; loop to try the lock again.  */
   } while (! __rtn);
  /* RTN is now nonzero; we have the lock.  */
  return __rtn;
}

/* Return nonzero if LOCK is locked.  */

_EXTERN_INLINE int
__spin_lock_locked (__spin_lock_t *__lock)
{
  return *__lock != 0;
}


#endif /* machine-lock.h */
