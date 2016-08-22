/* sem_timedwait -- wait on a semaphore with timeout.
   Copyright (C) 2003-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Paul Mackerras <paulus@au.ibm.com>, 2003.

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

#include "sem_waitcommon.c"

/* This is in a separate file because because sem_timedwait is only provided
   if __USE_XOPEN2K is defined.  */
int
sem_timedwait (sem_t *sem, const struct timespec *abstime)
{
  if (abstime->tv_nsec < 0 || abstime->tv_nsec >= 1000000000)
    {
      __set_errno (EINVAL);
      return -1;
    }

  /* Check sem_wait.c for a more detailed explanation why it is required.  */
  __pthread_testcancel ();

  if (__new_sem_wait_fast ((struct new_sem *) sem, 0) == 0)
    return 0;
  else
    return __new_sem_wait_slow((struct new_sem *) sem, abstime);
}
