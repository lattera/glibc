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

#include <errno.h>
#include <semaphore.h>
#include <lowlevellock.h>
#include <shlib-compat.h>
#include "semaphoreP.h"


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
  struct sem *isem = (struct sem *) sem;

  /* Use the value the user provided.  */
  isem->count = value;

  /* We can completely ignore the PSHARED parameter since inter-process
     use needs no special preparation.  */

  return 0;
}
versioned_symbol (libpthread, __new_sem_init, sem_init, GLIBC_2_1);
#if SHLIB_COMPAT(libpthread, GLIBC_2_0, GLIBC_2_1)
strong_alias (__new_sem_init, __old_sem_init)
compat_symbol (libpthread, __old_sem_init, sem_init, GLIBC_2_0);
#endif
