/* Linuxthreads - a simple clone()-based implementation of Posix        */
/* threads for Linux.                                                   */
/* Copyright (C) 1998 Xavier Leroy (Xavier.Leroy@inria.fr)              */
/*                                                                      */
/* This program is free software; you can redistribute it and/or        */
/* modify it under the terms of the GNU Library General Public License  */
/* as published by the Free Software Foundation; either version 2       */
/* of the License, or (at your option) any later version.               */
/*                                                                      */
/* This program is distributed in the hope that it will be useful,      */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of       */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        */
/* GNU Library General Public License for more details.                 */

#if defined(TEST_FOR_COMPARE_AND_SWAP)

extern int __pthread_has_cas;
extern int __pthread_compare_and_swap(long * ptr, long oldval, long newval,
                                      int * spinlock);

static inline int compare_and_swap(long * ptr, long oldval, long newval,
                                   int * spinlock)
{
  if (__builtin_expect (__pthread_has_cas, 1))
    return __compare_and_swap(ptr, oldval, newval);
  else
    return __pthread_compare_and_swap(ptr, oldval, newval, spinlock);
}

#elif defined(HAS_COMPARE_AND_SWAP)

static inline int compare_and_swap(long * ptr, long oldval, long newval,
                                   int * spinlock)
{
  return __compare_and_swap(ptr, oldval, newval);
}

#else

extern int __pthread_compare_and_swap(long * ptr, long oldval, long newval,
                                      int * spinlock);

static inline int compare_and_swap(long * ptr, long oldval, long newval,
                                   int * spinlock)
{
  return __pthread_compare_and_swap(ptr, oldval, newval, spinlock);
}

#endif

/* Internal locks */

extern void internal_function __pthread_lock(struct _pthread_fastlock * lock,
					     pthread_descr self);
extern void internal_function __pthread_unlock(struct _pthread_fastlock *lock);

static inline void __pthread_init_lock(struct _pthread_fastlock * lock)
{
  lock->__status = 0;
  lock->__spinlock = 0;
}

static inline int __pthread_trylock (struct _pthread_fastlock * lock)
{
  long oldstatus;

  do {
    oldstatus = lock->__status;
    if (oldstatus != 0) return EBUSY;
  } while(! compare_and_swap(&lock->__status, 0, 1, &lock->__spinlock));
  return 0;
}

#define LOCK_INITIALIZER {0, 0}
