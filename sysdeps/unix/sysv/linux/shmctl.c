/* Copyright (C) 1995,1997,1998,2000,2003,2004,2006
	Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <sys/shm.h>
#include <ipc_priv.h>

#include <sysdep.h>
#include <string.h>
#include <sys/syscall.h>
#include <bits/wordsize.h>
#include <shlib-compat.h>
#include <bp-checks.h>

#include <kernel-features.h>

struct __old_shmid_ds
{
  struct __old_ipc_perm shm_perm;	/* operation permission struct */
  int shm_segsz;			/* size of segment in bytes */
  __time_t shm_atime;			/* time of last shmat() */
  __time_t shm_dtime;			/* time of last shmdt() */
  __time_t shm_ctime;			/* time of last change by shmctl() */
  __ipc_pid_t shm_cpid;			/* pid of creator */
  __ipc_pid_t shm_lpid;			/* pid of last shmop */
  unsigned short int shm_nattch;	/* number of current attaches */
  unsigned short int __shm_npages;	/* size of segment (pages) */
  unsigned long int *__unbounded __shm_pages; /* array of ptrs to frames -> SHMMAX */
  struct vm_area_struct *__unbounded __attaches; /* descriptors for attaches */
};

struct __old_shminfo
{
  int shmmax;
  int shmmin;
  int shmmni;
  int shmseg;
  int shmall;
};

/* Provide operations to control over shared memory segments.  */
#if SHLIB_COMPAT (libc, GLIBC_2_0, GLIBC_2_2)
int __old_shmctl (int, int, struct __old_shmid_ds *);
#endif
int __new_shmctl (int, int, struct shmid_ds *);

#if SHLIB_COMPAT (libc, GLIBC_2_0, GLIBC_2_2)
int
attribute_compat_text_section
__old_shmctl (int shmid, int cmd, struct __old_shmid_ds *buf)
{
  return INLINE_SYSCALL (ipc, 5, IPCOP_shmctl, shmid,
			 cmd, 0, CHECK_1_NULL_OK (buf));
}
compat_symbol (libc, __old_shmctl, shmctl, GLIBC_2_0);
#endif

int
__new_shmctl (int shmid, int cmd, struct shmid_ds *buf)
{
#if __ASSUME_IPC64 > 0
  return INLINE_SYSCALL (ipc, 5, IPCOP_shmctl, shmid, cmd | __IPC_64, 0,
			 CHECK_1 (buf));
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
      return INLINE_SYSCALL (ipc, 5, IPCOP_shmctl, shmid, cmd, 0,
			     CHECK_1 (buf));
  }

  {
    int save_errno = errno, result;
    union
      {
	struct __old_shmid_ds ds;
	struct __old_shminfo info;
      } old;

    /* Unfortunately there is no way how to find out for sure whether
       we should use old or new shmctl.  */
    result = INLINE_SYSCALL (ipc, 5, IPCOP_shmctl, shmid, cmd | __IPC_64, 0,
			     CHECK_1 (buf));
    if (result != -1 || errno != EINVAL)
      return result;

    __set_errno(save_errno);
    if (cmd == IPC_SET)
      {
	old.ds.shm_perm.uid = buf->shm_perm.uid;
	old.ds.shm_perm.gid = buf->shm_perm.gid;
	old.ds.shm_perm.mode = buf->shm_perm.mode;
	if (old.ds.shm_perm.uid != buf->shm_perm.uid ||
	    old.ds.shm_perm.gid != buf->shm_perm.gid)
	  {
	    __set_errno (EINVAL);
	    return -1;
	  }
      }
    result = INLINE_SYSCALL (ipc, 5, IPCOP_shmctl, shmid, cmd, 0,
			     __ptrvalue (&old.ds));
    if (result != -1 && (cmd == SHM_STAT || cmd == IPC_STAT))
      {
	memset(buf, 0, sizeof(*buf));
	buf->shm_perm.__key = old.ds.shm_perm.__key;
	buf->shm_perm.uid = old.ds.shm_perm.uid;
	buf->shm_perm.gid = old.ds.shm_perm.gid;
	buf->shm_perm.cuid = old.ds.shm_perm.cuid;
	buf->shm_perm.cgid = old.ds.shm_perm.cgid;
	buf->shm_perm.mode = old.ds.shm_perm.mode;
	buf->shm_perm.__seq = old.ds.shm_perm.__seq;
	buf->shm_atime = old.ds.shm_atime;
	buf->shm_dtime = old.ds.shm_dtime;
	buf->shm_ctime = old.ds.shm_ctime;
	buf->shm_segsz = old.ds.shm_segsz;
	buf->shm_nattch = old.ds.shm_nattch;
	buf->shm_cpid = old.ds.shm_cpid;
	buf->shm_lpid = old.ds.shm_lpid;
      }
#if __WORDSIZE != 32
    else if (result != -1 && cmd == IPC_INFO)
      {
	struct shminfo *i = (struct shminfo *)buf;

	memset(i, 0, sizeof(*i));
	i->shmmax = old.info.shmmax;
	i->shmmin = old.info.shmmin;
	i->shmmni = old.info.shmmni;
	i->shmseg = old.info.shmseg;
	i->shmall = old.info.shmall;
      }
#endif
    return result;
  }
#endif
}

versioned_symbol (libc, __new_shmctl, shmctl, GLIBC_2_2);
