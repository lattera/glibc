/* Copyright (C) 1995-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, August 1995.

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
semop (int semid, struct sembuf *sops, size_t nsops)
{
#ifdef __ASSUME_DIRECT_SYSVIPC_SYSCALLS
  return INLINE_SYSCALL_CALL (semop, semid, sops, nsops);
#else
  return INLINE_SYSCALL_CALL (ipc, IPCOP_semop, semid, nsops, 0, sops);
#endif
}
