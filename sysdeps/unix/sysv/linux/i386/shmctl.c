/* Copyright (C) 1995-2012 Free Software Foundation, Inc.
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
#include <bp-checks.h>

#include <shlib-compat.h>

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
  return INLINE_SYSCALL (ipc, 5, IPCOP_shmctl,
			 shmid, cmd, 0, CHECK_1 (buf));
}
compat_symbol (libc, __old_shmctl, shmctl, GLIBC_2_0);
#endif

int
__new_shmctl (int shmid, int cmd, struct shmid_ds *buf)
{
  return INLINE_SYSCALL (ipc, 5, IPCOP_shmctl,
			 shmid, cmd | __IPC_64, 0, CHECK_1 (buf));
}

versioned_symbol (libc, __new_shmctl, shmctl, GLIBC_2_2);
