/* libc-internal interface for mutex locks.  Mach cthreads version.
Copyright (C) 1996 Free Software Foundation, Inc.
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

#ifndef _LIBC_LOCK_H
#define _LIBC_LOCK_H 1

#ifdef _LIBC
#include <cthreads.h>
#define __libc_lock_t struct mutex
#else
typedef struct __libc_lock_opaque__ __libc_lock_t;
#endif

/* Define a lock variable NAME with storage class CLASS.  The lock must be
   initialized with __libc_lock_init before it can be used (or define it
   with __libc_lock_define_initialized, below).  Use `extern' for CLASS to
   declare a lock defined in another module.  In public structure
   definitions, the lock element must come last, because its storage size
   will not be known outside of libc.  (Or you can use a pointer to the
   lock structure; i.e. NAME begins with a `*'.)  */
#define __libc_lock_define(CLASS,NAME) \
  CLASS __libc_lock_t NAME;

/* Define an initialized lock variable NAME with storage class CLASS.  */
#define __libc_lock_define_initialized(CLASS,NAME) \
  CLASS __libc_lock_t NAME = MUTEX_INITIALIZER;

/* Initialize the named lock variable, leaving it in a consistent, unlocked
   state.  */
#define __libc_lock_init(NAME) __mutex_init (&(NAME))

/* Finalize the named lock variable, which must be locked.  It cannot be
   used again until __libc_lock_init is called again on it.  This must be
   called on a lock variable before the containing storage is reused.  */
#define __libc_lock_fini(NAME) __mutex_unlock (&(NAME))

/* Lock the named lock variable.  */
#define __libc_lock_lock(NAME) __mutex_lock (&(NAME))

/* Unlock the named lock variable.  */
#define __libc_lock_unlock(NAME) __mutex_unlock (&(NAME))

/* Start a critical region with a cleanup function */
#define __libc_cleanup_region_start(FCT, ARG)				    \
{									    \
  (typeof FCT) __save_FCT = FCT;					    \
  (typeof ARG) __save_ARG = ARG;					    \
  /* close brace is in __libc_cleanup_region_end below. */

/* End a critical region started with __libc_cleanup_region_start. */
#define __libc_cleanup_region_end(DOIT)					    \
  if (DOIT)								    \
    (* __save_FCT)(__save_ARG);						    \
}

      

#endif	/* libc-lock.h */
