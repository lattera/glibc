/* Copyright (C) 1995-2015 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@gnu.ai.mit.edu>, August 1995.

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

#include <errno.h>
#include <sys/shm.h>
#include <ipc_priv.h>
#include <sysdep.h>
#include <sys/syscall.h>
#include <bits/wordsize.h>


int
__new_shmctl (int shmid, int cmd, struct shmid_ds *buf)
{
  return INLINE_SYSCALL (shmctl, 3, shmid, cmd | __IPC_64, buf);
}

#include <shlib-compat.h>
versioned_symbol (libc, __new_shmctl, shmctl, GLIBC_2_2);
