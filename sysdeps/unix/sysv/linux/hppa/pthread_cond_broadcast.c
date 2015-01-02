/* Copyright (C) 2009-2015 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Carlos O'Donell <carlos@codesourcery.com>, 2009.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef INCLUDED_SELF
# define INCLUDED_SELF
# include <pthread_cond_broadcast.c>
#else
# include <pthread.h>
# include <pthreadP.h>
# include <internaltypes.h>
# include <shlib-compat.h>
int
__pthread_cond_broadcast (cond)
     pthread_cond_t *cond;
{
  cond_compat_check_and_clear (cond);
  return __pthread_cond_broadcast_internal (cond);
}
versioned_symbol (libpthread, __pthread_cond_broadcast, pthread_cond_broadcast,
                  GLIBC_2_3_2);
# undef versioned_symbol
# define versioned_symbol(lib, local, symbol, version)
# undef __pthread_cond_broadcast
# define __pthread_cond_broadcast __pthread_cond_broadcast_internal
# include_next <pthread_cond_broadcast.c>
#endif
