/* libc-internal interface for mutex locks.  Hurd version using Mach cthreads.
   Copyright (C) 1996-2016 Free Software Foundation, Inc.
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

#ifndef _LIBC_LOCK_H
#define _LIBC_LOCK_H 1

#if (_LIBC - 0) || (_CTHREADS_ - 0)
#include <cthreads.h>
#include <hurd/threadvar.h>

/* The locking here is very inexpensive, even for inlining.  */
#define _IO_lock_inexpensive  1

typedef struct mutex __libc_lock_t;
typedef struct
{
  struct mutex mutex;
  void *owner;
  int count;
} __libc_lock_recursive_t;
typedef __libc_lock_recursive_t __rtld_lock_recursive_t;

#define __libc_lock_owner_self() ((void *) __hurd_threadvar_location (0))

#else
typedef struct __libc_lock_opaque__ __libc_lock_t;
typedef struct __libc_lock_recursive_opaque__ __libc_lock_recursive_t;
#endif

/* Define a lock variable NAME with storage class CLASS.  The lock must be
   initialized with __libc_lock_init before it can be used (or define it
   with __libc_lock_define_initialized, below).  Use `extern' for CLASS to
   declare a lock defined in another module.  In public structure
   definitions you must use a pointer to the lock structure (i.e., NAME
   begins with a `*'), because its storage size will not be known outside
   of libc.  */
#define __libc_lock_define(CLASS,NAME) \
  CLASS __libc_lock_t NAME;

/* Define an initialized lock variable NAME with storage class CLASS.  */
#define _LIBC_LOCK_INITIALIZER MUTEX_INITIALIZER
#define __libc_lock_define_initialized(CLASS,NAME) \
  CLASS __libc_lock_t NAME = _LIBC_LOCK_INITIALIZER;

/* Initialize the named lock variable, leaving it in a consistent, unlocked
   state.  */
#define __libc_lock_init(NAME) __mutex_init (&(NAME))

/* Finalize the named lock variable, which must be locked.  It cannot be
   used again until __libc_lock_init is called again on it.  This must be
   called on a lock variable before the containing storage is reused.  */
#define __libc_lock_fini(NAME) __mutex_unlock (&(NAME))
#define __libc_lock_fini_recursive(NAME) __mutex_unlock (&(NAME).mutex)
#define __rtld_lock_fini_recursive(NAME) __mutex_unlock (&(NAME).mutex)


/* Lock the named lock variable.  */
#define __libc_lock_lock(NAME) __mutex_lock (&(NAME))

/* Lock the named lock variable.  */
#define __libc_lock_trylock(NAME) (!__mutex_trylock (&(NAME)))

/* Unlock the named lock variable.  */
#define __libc_lock_unlock(NAME) __mutex_unlock (&(NAME))


#define __libc_lock_define_recursive(CLASS,NAME) \
  CLASS __libc_lock_recursive_t NAME;
#define _LIBC_LOCK_RECURSIVE_INITIALIZER { MUTEX_INITIALIZER, 0, 0 }
#define __libc_lock_define_initialized_recursive(CLASS,NAME) \
  CLASS __libc_lock_recursive_t NAME = _LIBC_LOCK_RECURSIVE_INITIALIZER;

#define __rtld_lock_define_recursive(CLASS,NAME) \
  __libc_lock_define_recursive (CLASS, NAME)
#define _RTLD_LOCK_RECURSIVE_INITIALIZER \
  _LIBC_LOCK_RECURSIVE_INITIALIZER
#define __rtld_lock_define_initialized_recursive(CLASS,NAME) \
  __libc_lock_define_initialized_recursive (CLASS, NAME)

#define __libc_lock_init_recursive(NAME) \
  ({ __libc_lock_recursive_t *const __lock = &(NAME); \
     __lock->owner = 0; mutex_init (&__lock->mutex); })

#define __libc_lock_trylock_recursive(NAME)				      \
  ({ __libc_lock_recursive_t *const __lock = &(NAME);			      \
     void *__self = __libc_lock_owner_self ();				      \
     __mutex_trylock (&__lock->mutex)					      \
     ? (__lock->owner = __self, __lock->count = 1, 0)			      \
     : __lock->owner == __self ? (++__lock->count, 0) : 1; })

#define __libc_lock_lock_recursive(NAME)				      \
  ({ __libc_lock_recursive_t *const __lock = &(NAME);			      \
     void *__self = __libc_lock_owner_self ();				      \
     if (__mutex_trylock (&__lock->mutex)				      \
	 || (__lock->owner != __self					      \
	     && (__mutex_lock (&__lock->mutex), 1)))			      \
       __lock->owner = __self, __lock->count = 1;			      \
     else								      \
       ++__lock->count;							      \
  })
#define __libc_lock_unlock_recursive(NAME)				      \
  ({ __libc_lock_recursive_t *const __lock = &(NAME);			      \
     if (--__lock->count == 0)						      \
       {								      \
	 __lock->owner = 0;						      \
	 __mutex_unlock (&__lock->mutex);				      \
       }								      \
  })


#define __rtld_lock_initialize(NAME) \
  (void) ((NAME) = (__rtld_lock_recursive_t) _RTLD_LOCK_RECURSIVE_INITIALIZER)
#define __rtld_lock_trylock_recursive(NAME) \
  __libc_lock_trylock_recursive (NAME)
#define __rtld_lock_lock_recursive(NAME) \
  __libc_lock_lock_recursive(NAME)
#define __rtld_lock_unlock_recursive(NAME) \
  __libc_lock_unlock_recursive (NAME)


/* XXX for now */
#define __libc_rwlock_define		__libc_lock_define
#define __libc_rwlock_define_initialized __libc_lock_define_initialized
#define __libc_rwlock_init		__libc_lock_init
#define __libc_rwlock_fini		__libc_lock_fini
#define __libc_rwlock_rdlock		__libc_lock_lock
#define __libc_rwlock_wrlock		__libc_lock_lock
#define __libc_rwlock_tryrdlock		__libc_lock_trylock
#define __libc_rwlock_trywrlock		__libc_lock_trylock
#define __libc_rwlock_unlock		__libc_lock_unlock


/* Start a critical region with a cleanup function */
#define __libc_cleanup_region_start(DOIT, FCT, ARG)			    \
{									    \
  typeof (***(FCT)) *__save_FCT = (DOIT) ? (FCT) : 0;			    \
  typeof (ARG) __save_ARG = ARG;					    \
  /* close brace is in __libc_cleanup_region_end below. */

/* End a critical region started with __libc_cleanup_region_start. */
#define __libc_cleanup_region_end(DOIT)					    \
  if ((DOIT) && __save_FCT != 0)					    \
    (*__save_FCT)(__save_ARG);						    \
}

/* Sometimes we have to exit the block in the middle.  */
#define __libc_cleanup_end(DOIT)					    \
  if ((DOIT) && __save_FCT != 0)					    \
    (*__save_FCT)(__save_ARG);						    \

#define __libc_cleanup_push(fct, arg) __libc_cleanup_region_start (1, fct, arg)
#define __libc_cleanup_pop(execute) __libc_cleanup_region_end (execute)

#if (_CTHREADS_ - 0)

/* Use mutexes as once control variables. */

struct __libc_once
  {
    __libc_lock_t lock;
    int done;
  };

#define __libc_once_define(CLASS,NAME) \
  CLASS struct __libc_once NAME = { MUTEX_INITIALIZER, 0 }

/* Call handler iff the first call.  */
#define __libc_once(ONCE_CONTROL, INIT_FUNCTION) \
  do {									      \
    __libc_lock_lock (ONCE_CONTROL.lock);				      \
    if (!ONCE_CONTROL.done)						      \
      (INIT_FUNCTION) ();						      \
    ONCE_CONTROL.done = 1;						      \
    __libc_lock_unlock (ONCE_CONTROL.lock);				      \
  } while (0)

/* Get once control variable.  */
#define __libc_once_get(ONCE_CONTROL)	((ONCE_CONTROL).done != 0)

#ifdef _LIBC
/* We need portable names for some functions.  E.g., when they are
   used as argument to __libc_cleanup_region_start.  */
#define __libc_mutex_unlock __mutex_unlock
#endif

/* Type for key of thread specific data.  */
typedef cthread_key_t __libc_key_t;

#define __libc_key_create(KEY,DEST) cthread_keycreate (KEY)
#define __libc_setspecific(KEY,VAL) cthread_setspecific (KEY, VAL)
void *__libc_getspecific (__libc_key_t key);

#endif /* _CTHREADS_ */

/* Hide the definitions which are only supposed to be used inside libc in
   a separate file.  This file is not present in the installation!  */
#ifdef _LIBC
# include <libc-lockP.h>
#endif

#endif	/* libc-lock.h */
