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

#include <limits.h>

#define SEM_VALUE_MAX INT_MAX

typedef struct {
  struct { long status; int spinlock; } sem_lock;
  int sem_value;
  _pthread_descr sem_waiting;
} sem_t;

__BEGIN_DECLS

extern int sem_init __P((sem_t *__sem, int __pshared, unsigned int __value));
extern int sem_destroy __P((sem_t *__sem));
extern int sem_wait __P((sem_t *__sem));
extern int sem_trywait __P((sem_t *__sem));
extern int sem_post __P((sem_t *__sem));
extern int sem_getvalue __P((sem_t *__sem, int *__sval));

__END_DECLS

#endif  /* semaphore.h */
