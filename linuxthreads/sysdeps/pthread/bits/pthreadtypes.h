/* Linuxthreads - a simple clone()-based implementation of Posix        */
/* threads for Linux.                                                   */
/* Copyright (C) 1996 Xavier Leroy (Xavier.Leroy@inria.fr)              */
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

#if !defined _BITS_TYPES_H && !defined _PTHREAD_H
# error "Never include <bits/pthreadtypes.h> directly; use <sys/types.h> instead."
#endif

#ifndef _BITS_PTHREADTYPES_H
#define _BITS_PTHREADTYPES_H	1

#define __need_schedparam
#include <bits/sched.h>

/* Fast locks (not abstract because mutexes and conditions aren't abstract). */
struct _pthread_fastlock
{
  long int status;              /* "Free" or "taken" or head of waiting list */
  int spinlock;                 /* For compare-and-swap emulation */
};

/* Thread descriptors */
typedef struct _pthread_descr_struct *_pthread_descr;


/* Attributes for threads.  */
typedef struct
{
  int detachstate;
  int schedpolicy;
  struct __sched_param schedparam;
  int inheritsched;
  int scope;
  size_t guardsize;
  int stackaddr_set;
  void *stackaddr;
  size_t stacksize;
} pthread_attr_t;


/* Conditions (not abstract because of PTHREAD_COND_INITIALIZER */
typedef struct
{
  struct _pthread_fastlock c_lock; /* Protect against concurrent access */
  _pthread_descr c_waiting;        /* Threads waiting on this condition */
} pthread_cond_t;


/* Attribute for conditionally variables.  */
typedef struct
{
  int dummy;
} pthread_condattr_t;

/* Keys for thread-specific data */
typedef unsigned int pthread_key_t;


/* Mutexes (not abstract because of PTHREAD_MUTEX_INITIALIZER).  */
/* (The layout is unnatural to maintain binary compatibility
    with earlier releases of LinuxThreads.) */
typedef struct
{
  int m_reserved;               /* Reserved for future use */
  int m_count;                  /* Depth of recursive locking */
  _pthread_descr m_owner;       /* Owner thread (if recursive or errcheck) */
  int m_kind;                   /* Mutex kind: fast, recursive or errcheck */
  struct _pthread_fastlock m_lock; /* Underlying fast lock */
} pthread_mutex_t;


/* Attribute for mutex.  */
typedef struct
{
  int mutexkind;
} pthread_mutexattr_t;


/* Once-only execution */
typedef int pthread_once_t;


#ifdef __USE_UNIX98
/* Read-write locks.  */
typedef struct
{
  struct _pthread_fastlock rw_lock; /* Lock to guarantee mutual exclusion */
  int rw_readers;               /* Number of readers */
  _pthread_descr rw_writer;     /* Identity of writer, or NULL if none */
  _pthread_descr rw_read_waiting; /* Threads waiting for reading */
  _pthread_descr rw_write_waiting; /* Threads waiting for writing */
  int rw_kind;                  /* Reader/Writer preference selection */
  int rw_pshared;               /* Shared between processes or not */
} pthread_rwlock_t;


/* Attribute for read-write locks.  */
typedef struct
{
  int lockkind;
  int pshared;
} pthread_rwlockattr_t;
#endif


/* Thread identifiers */
typedef unsigned long int pthread_t;

#endif	/* bits/pthreadtypes.h */
