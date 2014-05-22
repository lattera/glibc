/* Machine-specific pthread type layouts.  PowerPC version.
   Copyright (C) 2003-2017 Free Software Foundation, Inc.
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

#include <bits/wordsize.h>

#if __WORDSIZE == 64
# define __SIZEOF_PTHREAD_ATTR_T 56
# define __SIZEOF_PTHREAD_MUTEX_T 40
# define __SIZEOF_PTHREAD_MUTEXATTR_T 4
# define __SIZEOF_PTHREAD_COND_T 48
# define __SIZEOF_PTHREAD_CONDATTR_T 4
# define __SIZEOF_PTHREAD_RWLOCK_T 56
# define __SIZEOF_PTHREAD_RWLOCKATTR_T 8
# define __SIZEOF_PTHREAD_BARRIER_T 32
# define __SIZEOF_PTHREAD_BARRIERATTR_T 4
#else
# define __SIZEOF_PTHREAD_ATTR_T 36
# define __SIZEOF_PTHREAD_MUTEX_T 24
# define __SIZEOF_PTHREAD_MUTEXATTR_T 4
# define __SIZEOF_PTHREAD_COND_T 48
# define __SIZEOF_PTHREAD_CONDATTR_T 4
# define __SIZEOF_PTHREAD_RWLOCK_T 32
# define __SIZEOF_PTHREAD_RWLOCKATTR_T 8
# define __SIZEOF_PTHREAD_BARRIER_T 20
# define __SIZEOF_PTHREAD_BARRIERATTR_T 4
#endif


/* Thread identifiers.  The structure of the attribute type is
   deliberately not exposed.  */
typedef unsigned long int pthread_t;


union pthread_attr_t
{
  char __size[__SIZEOF_PTHREAD_ATTR_T];
  long int __align;
};
#ifndef __have_pthread_attr_t
typedef union pthread_attr_t pthread_attr_t;
# define __have_pthread_attr_t	1
#endif


#if __WORDSIZE == 64
typedef struct __pthread_internal_list
{
  struct __pthread_internal_list *__prev;
  struct __pthread_internal_list *__next;
} __pthread_list_t;
#else
typedef struct __pthread_internal_slist
{
  struct __pthread_internal_slist *__next;
} __pthread_slist_t;
#endif


/* Data structures for mutex handling.  The structure of the attribute
   type is deliberately not exposed.  */
typedef union
{
  struct __pthread_mutex_s
  {
    int __lock;
    unsigned int __count;
    int __owner;
#if __WORDSIZE == 64
    unsigned int __nusers;
#endif
    /* KIND must stay at this position in the structure to maintain
       binary compatibility with static initializers.  */
    int __kind;
#if __WORDSIZE == 64
    short __spins;
    short __elision;
    __pthread_list_t __list;
# define __PTHREAD_MUTEX_HAVE_PREV	1
# define __PTHREAD_SPINS             0, 0
#else
    unsigned int __nusers;
    __extension__ union
    {
      struct
      {
	short __espins;
	short __elision;
# define __spins __elision_data.__espins
# define __elision __elision_data.__elision
# define __PTHREAD_SPINS         { 0, 0 }
      } __elision_data;
      __pthread_slist_t __list;
    };
#endif
  } __data;
  char __size[__SIZEOF_PTHREAD_MUTEX_T];
  long int __align;
} pthread_mutex_t;

typedef union
{
  char __size[__SIZEOF_PTHREAD_MUTEXATTR_T];
  int __align;
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
   structure of the attribute type is deliberately not exposed.  */
typedef union
{
# if __WORDSIZE == 64
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
    unsigned char __rwelision;
    unsigned char __pad1[7];
    unsigned long int __pad2;
    /* FLAGS must stay at this position in the structure to maintain
       binary compatibility.  */
    unsigned int __flags;
# define __PTHREAD_RWLOCK_ELISION_EXTRA 0, {0, 0, 0, 0, 0, 0, 0 }
  } __data;
# else
  struct
  {
    unsigned int __readers;
    unsigned int __writers;
    unsigned int __wrphase_futex;
    unsigned int __writers_futex;
    unsigned int __pad3;
    unsigned int __pad4;
    unsigned char __rwelision;
    unsigned char __pad2;
    unsigned char __shared;
    /* FLAGS must stay at this position in the structure to maintain
       binary compatibility.  */
    unsigned char __flags;
    int __cur_writer;
#define __PTHREAD_RWLOCK_ELISION_EXTRA 0
  } __data;
# endif
  char __size[__SIZEOF_PTHREAD_RWLOCK_T];
  long int __align;
} pthread_rwlock_t;


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
