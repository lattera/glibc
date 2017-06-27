/* C11 threads thread-specific creation implementation.
   Copyright (C) 2018 Free Software Foundation, Inc.
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

#include "thrd_priv.h"

int
tss_create (tss_t *tss_id, tss_dtor_t destructor)
{
  _Static_assert (sizeof (tss_t) == sizeof (pthread_key_t),
		  "sizeof (tss_t) != sizeof (pthread_key_t)");
  _Static_assert (TSS_DTOR_ITERATIONS == PTHREAD_DESTRUCTOR_ITERATIONS,
		  "TSS_DTOR_ITERATIONS != PTHREAD_DESTRUCTOR_ITERATIONS");

  int err_code = __pthread_key_create (tss_id, destructor);
  return thrd_err_map (err_code);
}
