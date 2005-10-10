/* Copyright (C) 1995, 1997, 1998, 1999, 2000, 2005
   Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <errno.h>
#include <sys/sem.h>
#include <ipc_priv.h>
#include <alloca.h>
#include <sysdep.h>
#include <sys/syscall.h>
#include <bp-checks.h>

struct kernel_sembuf
{
  unsigned short int sem_num;   /* semaphore number */
  short int sem_op;             /* semaphore operation */
  short int sem_flg;            /* operation flag */
  short int __pad1;
};

/* Perform user-defined atomical operation of array of semaphores.  */

int
semtimedop (semid, sops, nsops, timeout)
     int semid;
     struct sembuf *sops;
     size_t nsops;
     const struct timespec *timeout;
{
  struct kernel_sembuf *ksops = alloca (sizeof (ksops[0]) * nsops);
  size_t i;
  int result;

  for (i = 0; i < nsops; i++)
    {
      ksops[i].sem_num = sops[i].sem_num;
      ksops[i].sem_op = sops[i].sem_op;
      ksops[i].sem_flg = sops[i].sem_flg;
    }

  result = INLINE_SYSCALL (ipc, 6, IPCOP_semtimedop,
			   semid, (int) nsops, 0, CHECK_N (sops, nsops),
			   timeout);

  for (i = 0; i < nsops; i++)
    {
      sops[i].sem_num = ksops[i].sem_num;
      sops[i].sem_op = ksops[i].sem_op;
      sops[i].sem_flg = ksops[i].sem_flg;
    }

  return result;
}
