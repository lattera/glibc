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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <semaphore.h>
#include <shlib-compat.h>
#include "semaphoreP.h"


int
__new_sem_destroy (sem)
     sem_t *sem;
{
  /* XXX Check for valid parameter.  */

  /* Nothing to do.  */
  return 0;
}
versioned_symbol (libpthread, __new_sem_destroy, sem_destroy, GLIBC_2_1);
#if SHLIB_COMPAT(libpthread, GLIBC_2_0, GLIBC_2_1)
strong_alias (__new_sem_destroy, __old_sem_destroy)
compat_symbol (libpthread, __old_sem_destroy, sem_destroy, GLIBC_2_0);
#endif
