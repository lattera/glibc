/* Initialize pid and tid fields of struct pthread.  Stub version.
   Copyright (C) 2015-2018 Free Software Foundation, Inc.
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

#include <pthreadP.h>

/* Initialize PD->pid and PD->tid for the initial thread.  If there is
   setup required to arrange that __exit_thread causes PD->tid to be
   cleared and futex-woken, then this function should do that as well.  */
static inline void
__pthread_initialize_pids (struct pthread *pd)
{
#error "sysdeps pthread-pids.h file required"
  pd->pid = pd->tid = -1;
}
