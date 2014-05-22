/* Copyright (C) 2002-2017 Free Software Foundation, Inc.

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

#ifndef _BITS_PTHREADTYPES_H
#define _BITS_PTHREADTYPES_H	1

#include <endian.h>

#ifdef __ILP32__
# define __SIZEOF_PTHREAD_ATTR_T        32
# define __SIZEOF_PTHREAD_MUTEX_T       32
# define __SIZEOF_PTHREAD_MUTEXATTR_T    4
# define __SIZEOF_PTHREAD_COND_T        48
# define __SIZEOF_PTHREAD_COND_COMPAT_T 48
# define __SIZEOF_PTHREAD_CONDATTR_T     4
# define __SIZEOF_PTHREAD_RWLOCK_T      48
# define __SIZEOF_PTHREAD_RWLOCKATTR_T   8
# define __SIZEOF_PTHREAD_BARRIER_T     20
# define __SIZEOF_PTHREAD_BARRIERATTR_T  4
#else
# define __SIZEOF_PTHREAD_ATTR_T        64
# define __SIZEOF_PTHREAD_MUTEX_T       48
# define __SIZEOF_PTHREAD_MUTEXATTR_T    8
# define __SIZEOF_PTHREAD_COND_T        48
# define __SIZEOF_PTHREAD_COND_COMPAT_T 48
# define __SIZEOF_PTHREAD_CONDATTR_T     8
# define __SIZEOF_PTHREAD_RWLOCK_T      56
# define __SIZEOF_PTHREAD_RWLOCKATTR_T   8
# define __SIZEOF_PTHREAD_BARRIER_T     32
# define __SIZEOF_PTHREAD_BARRIERATTR_T  8
#endif

#define __PTHREAD_RWLOCK_INT_FLAGS_SHARED 1

/* Thread identifiers.  The structure of the attribute type is not
   exposed on purpose.  */
typedef unsigned long int pthread_t;


union pthread_attr_t
{
  char __size[__SIZEOF_PTHREAD_ATTR_T];
  long int __align;
};
#ifndef __have_pthread_attr_t
typedef union pthread_attr_t pthread_attr_t;
# define __have_pthread_attr_t1
#endif

typedef struct __pthread_internal_list
{
  struct __pthread_internal_list *__prev;
  struct __pthread_internal_list *__next;
} __pthread_list_t;


/* Data structures for mutex handling.  The structure of the attribute
   type is not exposed on purpose.  */
typedef union
{
  struct __pthread_mutex_s
  {
    int __lock;
    unsigned int __count;
    int __owner;
    unsigned int __nusers;
    /* KIND must stay at this position in the structure to maintain
       binary compatibility with static initializers.  */
    int __kind;
    int __spins;
    __pthread_list_t __list;
#define __PTHREAD_MUTEX_HAVE_PREV	1
  } __data;
  char __size[__SIZEOF_PTHREAD_MUTEX_T];
  long int __align;
} pthread_mutex_t;

/* Mutex __spins initializer used by PTHREAD_MUTEX_INITIALIZER.  */
#define __PTHREAD_SPINS 0

typedef union
{
  char __size[__SIZEOF_PTHREAD_MUTEXATTR_T];
  long int __align;
} pthread_mutexattr_t;


/* Data structure for conditional variable handling.  The structure of
   the attribute type is not exposed on purpose.  */
typedef union
{
  struct
  {
    __extension__ union
    {
      __extension__ unsigned long long int __wseq;
      struct {
	unsigned int __low;
	unsigned int __high;
      } __wseq32;
    };
    __extension__ union
    {
      __extension__ unsigned long long int __g1_start;
      struct {
	unsigned int __low;
	unsigned int __high;
      } __g1_start32;
    };
    unsigned int __g_refs[2];
    unsigned int __g_size[2];
    unsigned int __g1_orig_size;
    unsigned int __wrefs;
    unsigned int __g_signals[2];
  } __data;
  char __size[__SIZEOF_PTHREAD_COND_T];
  __extension__ long long int __align;
} pthread_cond_t;

typedef union
{
  char __size[__SIZEOF_PTHREAD_CONDATTR_T];
  int __align;
} pthread_condattr_t;


/* Keys for thread-specific data */
typedef unsigned int pthread_key_t;


/* Once-only execution */
typedef int pthread_once_t;


#if defined __USE_UNIX98 || defined __USE_XOPEN2K
/* Data structure for read-write lock variable handling.  The
   structure of the attribute type is not exposed on purpose.  */
typedef union
{
  struct
  {
    unsigned int __readers;
    unsigned int __writers;
    unsigned int __wrphase_futex;
    unsigned int __writers_futex;
    unsigned int __pad3;
    unsigned int __pad4;
    int __cur_writer;
    int __shared;
    unsigned long int __pad1;
    unsigned long int __pad2;
    unsigned int __flags;
  } __data;
  char __size[__SIZEOF_PTHREAD_RWLOCK_T];
  long int __align;
} pthread_rwlock_t;

#define __PTHREAD_RWLOCK_ELISION_EXTRA 0

typedef union
{
  char __size[__SIZEOF_PTHREAD_RWLOCKATTR_T];
  long int __align;
} pthread_rwlockattr_t;
#endif


#ifdef __USE_XOPEN2K
/* POSIX spinlock data type.  */
typedef volatile int pthread_spinlock_t;


/* POSIX barriers data type.  The structure of the type is
   deliberately not exposed.  */
typedef union
{
  char __size[__SIZEOF_PTHREAD_BARRIER_T];
  long int __align;
} pthread_barrier_t;

typedef union
{
  char __size[__SIZEOF_PTHREAD_BARRIERATTR_T];
  int __align;
} pthread_barrierattr_t;
#endif

#endif	/* bits/pthreadtypes.h */
