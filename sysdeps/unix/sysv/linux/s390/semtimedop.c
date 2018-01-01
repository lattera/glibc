/* Copyright (C) 2003-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Martin Schwidefsky <schwidefsky@de.ibm.com>, 2003.

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

#include <sys/sem.h>
#include <ipc_priv.h>
#include <sysdep.h>
#include <errno.h>

/* Perform user-defined atomical operation of array of semaphores.  */

int
semtimedop (int semid, struct sembuf *sops, size_t nsops,
	    const struct timespec *timeout)
{
  /* The s390 sys_ipc variant has only five parameters instead of six
     (as for default variant) and the only difference is the handling of
     SEMTIMEDOP where on s390 the third parameter is used as a pointer
     to a struct timespec where the generic variant uses fifth parameter.  */
  return INLINE_SYSCALL_CALL (ipc, IPCOP_semtimedop, semid, nsops, timeout,
			      sops);
}
