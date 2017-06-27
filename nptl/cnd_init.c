/* C11 thread conditional initialization implementation.
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

#include <stdalign.h>

#include "thrd_priv.h"

int
cnd_init (cnd_t *cond)
{
  _Static_assert (sizeof (cnd_t) == sizeof (pthread_cond_t),
		  "(sizeof (cnd_t) != sizeof (pthread_cond_t)");
  _Static_assert (alignof (cnd_t) == alignof (pthread_cond_t),
		  "alignof (cnd_t) != alignof (pthread_cond_t)");

  int err_code = __pthread_cond_init ((pthread_cond_t *)cond, NULL);
  return thrd_err_map (err_code);
}
