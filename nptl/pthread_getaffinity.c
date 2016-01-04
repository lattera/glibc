/* Get the processor affinity of a thread.  Stub version.
   Copyright (C) 2014-2016 Free Software Foundation, Inc.
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

int
__pthread_getaffinity_np (pthread_t th, size_t cpusetsize, cpu_set_t *cpuset)
{
  const struct pthread *pd = (const struct pthread *) th;

  if (INVALID_TD_P (pd))
    return ESRCH;

  return ENOSYS;
}
weak_alias (__pthread_getaffinity_np, pthread_getaffinity_np)
stub_warning (pthread_getaffinity_np)
