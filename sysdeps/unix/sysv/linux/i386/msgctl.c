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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <sys/msg.h>
#include <ipc_priv.h>

#include <sysdep.h>
#include <string.h>
#include <sys/syscall.h>

#include <shlib-compat.h>

struct __old_msqid_ds
{
  struct __old_ipc_perm msg_perm;	/* structure describing operation permission */
  struct msg *__msg_first;		/* pointer to first message on queue */
  struct msg *__msg_last;		/* pointer to last message on queue */
  __time_t msg_stime;			/* time of last msgsnd command */
  __time_t msg_rtime;			/* time of last msgrcv command */
  __time_t msg_ctime;			/* time of last change */
  struct wait_queue *__wwait;		/* ??? */
  struct wait_queue *__rwait;		/* ??? */
  unsigned short int __msg_cbytes;	/* current number of bytes on queue */
  unsigned short int msg_qnum;		/* number of messages currently on queue */
  unsigned short int msg_qbytes;	/* max number of bytes allowed on queue */
  __ipc_pid_t msg_lspid;		/* pid of last msgsnd() */
  __ipc_pid_t msg_lrpid;		/* pid of last msgrcv() */
};

/* Allows to control internal state and destruction of message queue
   objects.  */
#if SHLIB_COMPAT (libc, GLIBC_2_0, GLIBC_2_2)
int __old_msgctl (int, int, struct __old_msqid_ds *);
#endif
int __new_msgctl (int, int, struct msqid_ds *);

#if SHLIB_COMPAT (libc, GLIBC_2_0, GLIBC_2_2)
int
attribute_compat_text_section
__old_msgctl (int msqid, int cmd, struct __old_msqid_ds *buf)
{
  return INLINE_SYSCALL (ipc, 5, IPCOP_msgctl, msqid, cmd, 0, buf);
}
compat_symbol (libc, __old_msgctl, msgctl, GLIBC_2_0);
#endif

int
__new_msgctl (int msqid, int cmd, struct msqid_ds *buf)
{
  return INLINE_SYSCALL (ipc, 5, IPCOP_msgctl,
			 msqid, cmd | __IPC_64, 0, buf);
}

versioned_symbol (libc, __new_msgctl, msgctl, GLIBC_2_2);
