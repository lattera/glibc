/* Copyright (C) 2002 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2002.

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

#include "pthreadP.h"

/* Mount point of the shared memory filesystem.  */
struct mountpoint_info
{
  char *dir;
  size_t dirlen;
};


/* Variables used in multiple interfaces.  */
extern struct mountpoint_info mountpoint attribute_hidden;

extern pthread_once_t __namedsem_once attribute_hidden;


/* Initializer for mountpoint.  */
extern void __where_is_shmfs (void) attribute_hidden;


/* Prototypes of functions with multiple interfaces.  */
extern int __new_sem_init (sem_t *sem, int pshared, unsigned int value);
extern int __new_sem_destroy (sem_t *sem);
extern int __new_sem_post (sem_t *sem);
extern int __new_sem_wait (sem_t *sem);
extern int __new_sem_trywait (sem_t *sem);
extern int __new_sem_getvalue (sem_t *sem, int *sval);
