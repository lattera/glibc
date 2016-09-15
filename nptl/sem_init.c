/* Copyright (C) 2002-2016 Free Software Foundation, Inc.
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
#include <semaphore.h>
#include <shlib-compat.h>
#include "semaphoreP.h"
#include <kernel-features.h>
#include <futex-internal.h>


int
__new_sem_init (sem_t *sem, int pshared, unsigned int value)
{
  /* Parameter sanity check.  */
  if (__glibc_unlikely (value > SEM_VALUE_MAX))
    {
      __set_errno (EINVAL);
      return -1;
    }
  pshared = pshared != 0 ? PTHREAD_PROCESS_SHARED : PTHREAD_PROCESS_PRIVATE;
  int err = futex_supports_pshared (pshared);
  if (err != 0)
    {
      __set_errno (err);
      return -1;
    }

  /* Map to the internal type.  */
  struct new_sem *isem = (struct new_sem *) sem;

  /* Use the values the caller provided.  */
#if __HAVE_64B_ATOMICS
  isem->data = value;
#else
  isem->value = value << SEM_VALUE_SHIFT;
  /* pad is used as a mutex on pre-v9 sparc and ignored otherwise.  */
  isem->pad = 0;
  isem->nwaiters = 0;
#endif

  isem->private = (pshared == PTHREAD_PROCESS_PRIVATE
		   ? FUTEX_PRIVATE : FUTEX_SHARED);

  return 0;
}
versioned_symbol (libpthread, __new_sem_init, sem_init, GLIBC_2_1);



#if SHLIB_COMPAT(libpthread, GLIBC_2_0, GLIBC_2_1)
int
attribute_compat_text_section
__old_sem_init (sem_t *sem, int pshared, unsigned int value)
{
  /* Parameter sanity check.  */
  if (__glibc_unlikely (value > SEM_VALUE_MAX))
    {
      __set_errno (EINVAL);
      return -1;
    }

  /* Map to the internal type.  */
  struct old_sem *isem = (struct old_sem *) sem;

  /* Use the value the user provided.  */
  isem->value = value;

  /* We cannot store the PSHARED attribute.  So we always use the
     operations needed for shared semaphores.  */

  return 0;
}
compat_symbol (libpthread, __old_sem_init, sem_init, GLIBC_2_0);
#endif
