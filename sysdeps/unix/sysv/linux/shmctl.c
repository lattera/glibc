/* Copyright (C) 1995, 1997, 1998, 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@gnu.ai.mit.edu>, August 1995.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#define __LIBC_IPC_INTERNAL
#include <errno.h>
#include <sys/shm.h>

#include <sysdep.h>
#include <string.h>
#include <sys/syscall.h>
#include <bits/wordsize.h>

#include "kernel-features.h"

/* Provide operations to control over shared memory segments.  */
int __old_shmctl (int, int, struct __old_shmid_ds *);
int __new_shmctl (int, int, struct shmid_ds *);

int
__old_shmctl (int shmid, int cmd, struct __old_shmid_ds *buf)
{
  return INLINE_SYSCALL (ipc, 5, IPCOP_shmctl, shmid, cmd, 0, buf);
}

int
__new_shmctl (int shmid, int cmd, struct shmid_ds *buf)
{
#if __ASSUME_32BITUIDS > 0
  return INLINE_SYSCALL (ipc, 5, IPCOP_shmctl, shmid, cmd | __IPC_64, 0, buf);
#else
  switch (cmd) {
    case SHM_STAT:
    case IPC_STAT:
    case IPC_SET:
#if __WORDSIZE != 32
    case IPC_INFO:
#endif
      break;
    default:
      return INLINE_SYSCALL (ipc, 5, IPCOP_shmctl, shmid, cmd, 0, buf);
  }

  {
    int save_errno = errno, result;
    struct __old_shmid_ds old;

    /* Unfortunately there is no way how to find out for sure whether
       we should use old or new shmctl.  */
    result = INLINE_SYSCALL (ipc, 5, IPCOP_shmctl, shmid, cmd | __IPC_64, 0, buf);
    if (result != -1 || errno != EINVAL)
      return result;

    __set_errno(save_errno);
    if (cmd == IPC_SET)
      {
	old.shm_perm.uid = buf->shm_perm.uid;
	old.shm_perm.gid = buf->shm_perm.gid;
	old.shm_perm.mode = buf->shm_perm.mode;
	if (old.shm_perm.uid != buf->shm_perm.uid ||
	    old.shm_perm.gid != buf->shm_perm.gid)
	  {
	    __set_errno (EINVAL);
	    return -1;
	  }
      }
    result = INLINE_SYSCALL (ipc, 5, IPCOP_shmctl, shmid, cmd, 0, &old);
    if (result != -1 && (cmd == SHM_STAT || cmd == IPC_STAT))
      {
	memset(buf, 0, sizeof(*buf));
	buf->shm_perm.__key = old.shm_perm.__key;
	buf->shm_perm.uid = old.shm_perm.uid;
	buf->shm_perm.gid = old.shm_perm.gid;
	buf->shm_perm.cuid = old.shm_perm.cuid;
	buf->shm_perm.cgid = old.shm_perm.cgid;
	buf->shm_perm.mode = old.shm_perm.mode;
	buf->shm_perm.__seq = old.shm_perm.__seq;
	buf->shm_atime = old.shm_atime;
	buf->shm_dtime = old.shm_dtime;
	buf->shm_ctime = old.shm_ctime;
	buf->shm_segsz = old.shm_segsz;
	buf->shm_nattch = old.shm_nattch;
	buf->shm_cpid = old.shm_cpid;
	buf->shm_lpid = old.shm_lpid;
      }
#if __WORDSIZE != 32
    else if (result != -1 && cmd == IPC_INFO)
      {
	struct __old_shminfo *oldi = (struct __old_shminfo *)&old;
	struct shminfo *i = (struct shminfo *)buf;

	memset(i, 0, sizeof(*i));
	i->shmmax = oldi->shmmax;
	i->shmmin = oldi->shmmin;
	i->shmmni = oldi->shmmni;
	i->shmseg = oldi->shmseg;
	i->shmall = oldi->shmall;
      }
#endif
    return result;
  }
#endif
}

#if defined PIC && DO_VERSIONING
default_symbol_version (__new_shmctl, shmctl, GLIBC_2.2);
symbol_version (__old_shmctl, shmctl, GLIBC_2.0);
#else
weak_alias (__new_shmctl, shmctl);
#endif
