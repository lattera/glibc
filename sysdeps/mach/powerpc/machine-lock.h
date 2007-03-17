/* Machine-specific definition for spin locks.  PowerPC version.
   Copyright (C) 1994,97,2002,2007 Free Software Foundation, Inc.
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

/* The type of a spin lock variable.  */

typedef __volatile long int __spin_lock_t;

/* Value to initialize `__spin_lock_t' variables to.  */

#define	__SPIN_LOCK_INITIALIZER	0L


#ifndef _EXTERN_INLINE
#define _EXTERN_INLINE __extern_inline
#endif

/* Unlock LOCK.  */

_EXTERN_INLINE void
__spin_unlock (__spin_lock_t *__lock)
{
  long int __locked;
  __asm__ __volatile__ ("\
0:	lwarx	%0,0,%1\n\
	stwcx.	%2,0,%1\n\
	bne-	0b\n\
" : "=&r" (__locked) : "r" (__lock), "r" (0) : "cr0");
}

/* Try to lock LOCK; return nonzero if we locked it, zero if another has.  */

_EXTERN_INLINE int
__spin_try_lock (register __spin_lock_t *__lock)
{
  long int __rtn;
  __asm__ __volatile__ ("\
0:	lwarx	%0,0,%1\n\
	stwcx.	%2,0,%1\n\
	bne-	0b\n\
" : "=&r" (__rtn) : "r" (__lock), "r" (1) : "cr0");
  return !__rtn;
}

/* Return nonzero if LOCK is locked.  */

_EXTERN_INLINE int
__spin_lock_locked (__spin_lock_t *__lock)
{
  long int __rtn;
  __asm__ __volatile__ ("\
0:	lwarx	%0,0,%1\n\
	stwcx.	%0,0,%1\n\
	bne-	0b\n\
" : "=&r" (__rtn) : "r" (__lock) : "cr0");
  return __rtn;
}


#endif /* machine-lock.h */
