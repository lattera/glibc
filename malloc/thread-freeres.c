/* Free resources stored in thread-local variables on thread exit.
   Copyright (C) 2003-2015 Free Software Foundation, Inc.
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

#include <stdlib.h>
#include <libc-internal.h>
#include <set-hooks.h>

#ifdef _LIBC_REENTRANT
DEFINE_HOOK (__libc_thread_subfreeres, (void));

void __attribute__ ((section ("__libc_thread_freeres_fn")))
__libc_thread_freeres (void)
{
  RUN_HOOK (__libc_thread_subfreeres, ());
}
#endif
