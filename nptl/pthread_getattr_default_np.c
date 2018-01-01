/* Get the default attributes used by pthread_create in the process.
   Copyright (C) 2013-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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
#include <stdlib.h>
#include <pthreadP.h>

int
pthread_getattr_default_np (pthread_attr_t *out)
{
  struct pthread_attr *real_out;

  real_out = (struct pthread_attr *) out;

  lll_lock (__default_pthread_attr_lock, LLL_PRIVATE);
  *real_out = __default_pthread_attr;
  lll_unlock (__default_pthread_attr_lock, LLL_PRIVATE);

  return 0;
}
