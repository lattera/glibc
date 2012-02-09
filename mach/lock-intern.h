/* Copyright (C) 1994, 1996, 2007 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef _LOCK_INTERN_H
#define	_LOCK_INTERN_H

#include <sys/cdefs.h>
#include <machine-lock.h>

#ifndef _EXTERN_INLINE
#define _EXTERN_INLINE __extern_inline
#endif


/* Initialize LOCK.  */

_EXTERN_INLINE void
__spin_lock_init (__spin_lock_t *__lock)
{
  *__lock = __SPIN_LOCK_INITIALIZER;
}


/* Lock LOCK, blocking if we can't get it.  */
extern void __spin_lock_solid (__spin_lock_t *__lock);

/* Lock the spin lock LOCK.  */

_EXTERN_INLINE void
__spin_lock (__spin_lock_t *__lock)
{
  if (! __spin_try_lock (__lock))
    __spin_lock_solid (__lock);
}

/* Name space-clean internal interface to mutex locks.

   Code internal to the C library uses these functions to lock and unlock
   mutex locks.  These locks are of type `struct mutex', defined in
   <cthreads.h>.  The functions here are name space-clean.  If the program
   is linked with the cthreads library, `__mutex_lock_solid' and
   `__mutex_unlock_solid' will invoke the corresponding cthreads functions
   to implement real mutex locks.  If not, simple stub versions just use
   spin locks.  */


/* Initialize the newly allocated mutex lock LOCK for further use.  */
extern void __mutex_init (void *__lock);

/* Lock LOCK, blocking if we can't get it.  */
extern void __mutex_lock_solid (void *__lock);

/* Finish unlocking LOCK, after the spin lock LOCK->held has already been
   unlocked.  This function will wake up any thread waiting on LOCK.  */
extern void __mutex_unlock_solid (void *__lock);

/* Lock the mutex lock LOCK.  */

_EXTERN_INLINE void
__mutex_lock (void *__lock)
{
  if (! __spin_try_lock ((__spin_lock_t *) __lock))
    __mutex_lock_solid (__lock);
}

/* Unlock the mutex lock LOCK.  */

_EXTERN_INLINE void
__mutex_unlock (void *__lock)
{
  __spin_unlock ((__spin_lock_t *) __lock);
  __mutex_unlock_solid (__lock);
}


_EXTERN_INLINE int
__mutex_trylock (void *__lock)
{
  return __spin_try_lock ((__spin_lock_t *) __lock);
}

#endif /* lock-intern.h */
