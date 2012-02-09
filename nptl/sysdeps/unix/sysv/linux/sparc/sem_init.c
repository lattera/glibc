/* Copyright (C) 2002, 2007 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <string.h>
#include <semaphore.h>
#include <lowlevellock.h>
#include <shlib-compat.h>
#include "semaphoreP.h"
#include <kernel-features.h>


int
__new_sem_init (sem, pshared, value)
     sem_t *sem;
     int pshared;
     unsigned int value;
{
  /* Parameter sanity check.  */
  if (__builtin_expect (value > SEM_VALUE_MAX, 0))
    {
      __set_errno (EINVAL);
      return -1;
    }

  /* Map to the internal type.  */
  struct sparc_new_sem *isem = (struct sparc_new_sem *) sem;

  /* Use the values the user provided.  */
  memset (isem, '\0', sizeof (*isem));
  isem->value = value;
#ifdef __ASSUME_PRIVATE_FUTEX
  isem->private = pshared ? 0 : FUTEX_PRIVATE_FLAG;
#else
  isem->private = pshared ? 0 : THREAD_GETMEM (THREAD_SELF,
					       header.private_futex);
#endif

  return 0;
}
versioned_symbol (libpthread, __new_sem_init, sem_init, GLIBC_2_1);



#if SHLIB_COMPAT(libpthread, GLIBC_2_0, GLIBC_2_1)
int
attribute_compat_text_section
__old_sem_init (sem, pshared, value)
     sem_t *sem;
     int pshared;
     unsigned int value;
{
  /* Parameter sanity check.  */
  if (__builtin_expect (value > SEM_VALUE_MAX, 0))
    {
      __set_errno (EINVAL);
      return -1;
    }

  /* Map to the internal type.  */
  struct sparc_old_sem *isem = (struct sparc_old_sem *) sem;

  /* Use the value the user provided.  */
  memset (isem, '\0', sizeof (*isem));
  isem->value = value;

#ifdef __ASSUME_PRIVATE_FUTEX
  isem->private = pshared ? 0 : FUTEX_PRIVATE_FLAG;
#else
  isem->private = pshared ? 0 : THREAD_GETMEM (THREAD_SELF,
					       header.private_futex);
#endif

  return 0;
}
compat_symbol (libpthread, __old_sem_init, sem_init, GLIBC_2_0);
#endif
