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


int
__pthread_cond_init (cond, cond_attr)
     pthread_cond_t *cond;
     const pthread_condattr_t *cond_attr;
{
  /* Note that we don't need the COND-ATTR.  It contains only the
     PSHARED flag which is unimportant here since conditional
     variables are always usable in multiple processes.  */

  cond->__data.__lock = LLL_MUTEX_LOCK_INITIALIZER;
  cond->__data.__nr_wakers = 0;
  cond->__data.__nr_sleepers = 0;

  return 0;
}
strong_alias (__pthread_cond_init, pthread_cond_init)
