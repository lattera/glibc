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

#ifndef _SEMAPHORE_H
#define _SEMAPHORE_H    1

#include <features.h>
#include <sys/types.h>

#ifndef _PTHREAD_DESCR_DEFINED
/* Thread descriptors.  Needed for `sem_t' definition.  */
typedef struct _pthread_descr_struct *_pthread_descr;
# define _PTHREAD_DESCR_DEFINED
#endif

/* System specific semaphore definition.  */
typedef struct
{
  struct
  {
    long int status;
    int spinlock;
  } __sem_lock;
  int __sem_value;
  _pthread_descr __sem_waiting;
} sem_t;



/* Value returned if `sem_open' failed.  */
#define SEM_FAILED	((sem_t *) NULL)

/* Maximum value the semaphore can have.  */
#define SEM_VALUE_MAX 	((int) ((~0u) >> 1))


__BEGIN_DECLS

/* Initialize semaphore object SEM to VALUE.  If PSHARED then share it
   with other processes.  */
extern int sem_init __P ((sem_t *__sem, int __pshared, unsigned int __value));

/* Free resources associated with semaphore object SEM.  */
extern int sem_destroy __P ((sem_t *__sem));

/* Open a named semaphore NAME with open flaot OFLAG.  */
extern sem_t *sem_open __P ((__const char *__name, int __oflag, ...));

/* Close descriptor for named semaphore SEM.  */
extern int sem_close __P ((sem_t *__sem));

/* Remove named semaphore NAME.  */
extern int sem_unlink __P ((__const char *__name));

/* Wait for SEM being posted.  */
extern int sem_wait __P ((sem_t *__sem));

/* Test whether SEM is posted.  */
extern int sem_trywait __P ((sem_t *__sem));

/* Post SEM.  */
extern int sem_post __P ((sem_t *__sem));

/* Get current value of SEM and store it in *SVAL.  */
extern int sem_getvalue __P ((sem_t *__sem, int *__sval));

__END_DECLS

#endif  /* semaphore.h */
