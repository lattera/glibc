/* Copyright (C) 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2003.

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
#include <pthreadP.h>
#include <string.h>
#include <sysdep.h>
#include <sys/types.h>


int
pthread_getaffinity_np (th, cpuset)
     pthread_t th;
     cpu_set_t *cpuset;
{
  struct pthread *pd = (struct pthread *) th;
  INTERNAL_SYSCALL_DECL (err);
  int res = INTERNAL_SYSCALL (sched_getaffinity, err, 3, pd->tid,
			      sizeof (cpu_set_t), cpuset);
  if (INTERNAL_SYSCALL_ERROR_P (res, err))
    return INTERNAL_SYSCALL_ERRNO (res, err);

  /* Clean the rest of the memory the kernel didn't do.  */
  memset ((char *) cpuset + res, '\0', sizeof (cpu_set_t) - res);

  return 0;
}
