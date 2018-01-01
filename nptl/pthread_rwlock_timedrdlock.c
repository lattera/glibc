/* Copyright (C) 2003-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Martin Schwidefsky <schwidefsky@de.ibm.com>, 2003.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include "pthread_rwlock_common.c"

/* See pthread_rwlock_common.c.  */
int
pthread_rwlock_timedrdlock (pthread_rwlock_t *rwlock,
    const struct timespec *abstime)
{
  /* Make sure the passed in timeout value is valid.  Note that the previous
     implementation assumed that this check *must* not be performed if there
     would in fact be no blocking; however, POSIX only requires that "the
     validity of the abstime parameter need not be checked if the lock can be
     immediately acquired" (i.e., we need not but may check it).  */
  /* ??? Just move this to __pthread_rwlock_rdlock_full?  */
  if (__glibc_unlikely (abstime->tv_nsec >= 1000000000
      || abstime->tv_nsec < 0))
    return EINVAL;

  return __pthread_rwlock_rdlock_full (rwlock, abstime);
}
