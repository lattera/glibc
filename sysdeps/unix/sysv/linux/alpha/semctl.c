/* Copyright (C) 1995,1997,1998,2000,2003,2006
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <errno.h>
#include <stdarg.h>
#include <sys/sem.h>
#include <ipc_priv.h>

#include <sysdep.h>
#include <string.h>
#include <sys/syscall.h>

#include <kernel-features.h>

struct __old_semid_ds
{
  struct __old_ipc_perm sem_perm;	/* operation permission struct */
  __time_t sem_otime;			/* last semop() time */
  __time_t sem_ctime;			/* last time changed by semctl() */
  struct sem *__sembase;		/* ptr to first semaphore in array */
  struct sem_queue *__sem_pending;	/* pending operations */
  struct sem_queue *__sem_pending_last; /* last pending operation */
  struct sem_undo *__undo;		/* ondo requests on this array */
  unsigned short int sem_nsems;		/* number of semaphores in set */
};

/* Define a `union semun' suitable for Linux here.  */
union semun
{
  int val;			/* value for SETVAL */
  struct semid_ds *buf;		/* buffer for IPC_STAT & IPC_SET */
  unsigned short int *array;	/* array for GETALL & SETALL */
  struct seminfo *__buf;	/* buffer for IPC_INFO */
};

#include <bp-checks.h>
#include <bp-semctl.h>		/* definition of CHECK_SEMCTL needs union semum */

/* Return identifier for array of NSEMS semaphores associated with
   KEY.  */
int __new_semctl (int semid, int semnum, int cmd, ...);

int
__new_semctl (int semid, int semnum, int cmd, ...)
{
  union semun arg;
  va_list ap;

  va_start (ap, cmd);

  /* Get the argument.  */
  arg = va_arg (ap, union semun);

  va_end (ap);

#if __ASSUME_32BITUIDS > 0
  return INLINE_SYSCALL (semctl, 4, semid, semnum, cmd | __IPC_64,
			 CHECK_SEMCTL (&arg, semid, cmd | __IPC_64)->array);
#else
  switch (cmd) {
    case SEM_STAT:
    case IPC_STAT:
    case IPC_SET:
      break;
    default:
      return INLINE_SYSCALL (semctl, 4, semid, semnum, cmd,
			     CHECK_SEMCTL (&arg, semid, cmd)->array);
  }

  {
    int save_errno = errno, result;
    struct __old_semid_ds old;
    struct semid_ds *buf;

    /* Unfortunately there is no way how to find out for sure whether
       we should use old or new semctl.  */
    result = INLINE_SYSCALL (semctl, 4, semid, semnum, cmd | __IPC_64,
			     CHECK_SEMCTL (&arg, semid, cmd | __IPC_64)->array);
    if (result != -1 || errno != EINVAL)
      return result;

    __set_errno(save_errno);
    buf = arg.buf;
    arg.buf = (void *)&old;
    if (cmd == IPC_SET)
      {
	old.sem_perm.uid = buf->sem_perm.uid;
	old.sem_perm.gid = buf->sem_perm.gid;
	old.sem_perm.mode = buf->sem_perm.mode;
	if (old.sem_perm.uid != buf->sem_perm.uid ||
	    old.sem_perm.gid != buf->sem_perm.gid)
	  {
	    __set_errno (EINVAL);
	    return -1;
	  }
      }
    result = INLINE_SYSCALL (semctl, 4, semid, semnum, cmd,
			     CHECK_SEMCTL (&arg, semid, cmd)->array);
    if (result != -1 && cmd != IPC_SET)
      {
	memset(buf, 0, sizeof(*buf));
	buf->sem_perm.__key = old.sem_perm.__key;
	buf->sem_perm.uid = old.sem_perm.uid;
	buf->sem_perm.gid = old.sem_perm.gid;
	buf->sem_perm.cuid = old.sem_perm.cuid;
	buf->sem_perm.cgid = old.sem_perm.cgid;
	buf->sem_perm.mode = old.sem_perm.mode;
	buf->sem_perm.__seq = old.sem_perm.__seq;
	buf->sem_otime = old.sem_otime;
	buf->sem_ctime = old.sem_ctime;
	buf->sem_nsems = old.sem_nsems;
      }
    return result;
  }
#endif
}

#include <shlib-compat.h>
versioned_symbol (libc, __new_semctl, semctl, GLIBC_2_2);
