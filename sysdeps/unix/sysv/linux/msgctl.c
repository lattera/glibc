/* Copyright (C) 1995, 1997, 1998, 2000, 2002, 2004
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
#include <sys/msg.h>
#include <ipc_priv.h>

#include <sysdep.h>
#include <string.h>
#include <sys/syscall.h>
#include <shlib-compat.h>
#include <bp-checks.h>

#include "kernel-features.h"

struct __old_msqid_ds
{
  struct __old_ipc_perm msg_perm;	/* structure describing operation permission */
  struct msg *__unbounded __msg_first;	/* pointer to first message on queue */
  struct msg *__unbounded __msg_last;	/* pointer to last message on queue */
  __time_t msg_stime;			/* time of last msgsnd command */
  __time_t msg_rtime;			/* time of last msgrcv command */
  __time_t msg_ctime;			/* time of last change */
  struct wait_queue *__unbounded __wwait; /* ??? */
  struct wait_queue *__unbounded __rwait; /* ??? */
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
  return INLINE_SYSCALL (ipc, 5, IPCOP_msgctl, msqid, cmd, 0, CHECK_1 (buf));
}
compat_symbol (libc, __old_msgctl, msgctl, GLIBC_2_0);
#endif

int
__new_msgctl (int msqid, int cmd, struct msqid_ds *buf)
{
#if __ASSUME_IPC64 > 0
  return INLINE_SYSCALL (ipc, 5, IPCOP_msgctl,
			 msqid, cmd | __IPC_64, 0, CHECK_1 (buf));
#else
  switch (cmd) {
    case MSG_STAT:
    case IPC_STAT:
    case IPC_SET:
      break;
    default:
      return INLINE_SYSCALL (ipc, 5, IPCOP_msgctl,
			     msqid, cmd, 0, CHECK_1 (buf));
  }

  {
    int result;
    struct __old_msqid_ds old;

    /* Unfortunately there is no way how to find out for sure whether
       we should use old or new msgctl.  */
    result = INLINE_SYSCALL (ipc, 5, IPCOP_msgctl,
			     msqid, cmd | __IPC_64, 0, CHECK_1 (buf));
    if (result != -1 || errno != EINVAL)
      return result;

    if (cmd == IPC_SET)
      {
	old.msg_perm.uid = buf->msg_perm.uid;
	old.msg_perm.gid = buf->msg_perm.gid;
	old.msg_perm.mode = buf->msg_perm.mode;
	old.msg_qbytes = buf->msg_qbytes;
	if (old.msg_perm.uid != buf->msg_perm.uid ||
	    old.msg_perm.gid != buf->msg_perm.gid ||
	    old.msg_qbytes != buf->msg_qbytes)
	  {
	    __set_errno (EINVAL);
	    return -1;
	  }
      }
    result = INLINE_SYSCALL (ipc, 5, IPCOP_msgctl,
			     msqid, cmd, 0, __ptrvalue (&old));
    if (result != -1 && cmd != IPC_SET)
      {
	memset(buf, 0, sizeof(*buf));
	buf->msg_perm.__key = old.msg_perm.__key;
	buf->msg_perm.uid = old.msg_perm.uid;
	buf->msg_perm.gid = old.msg_perm.gid;
	buf->msg_perm.cuid = old.msg_perm.cuid;
	buf->msg_perm.cgid = old.msg_perm.cgid;
	buf->msg_perm.mode = old.msg_perm.mode;
	buf->msg_perm.__seq = old.msg_perm.__seq;
	buf->msg_stime = old.msg_stime;
	buf->msg_rtime = old.msg_rtime;
	buf->msg_ctime = old.msg_ctime;
	buf->__msg_cbytes = old.__msg_cbytes;
	buf->msg_qnum = old.msg_qnum;
	buf->msg_qbytes = old.msg_qbytes;
	buf->msg_lspid = old.msg_lspid;
	buf->msg_lrpid = old.msg_lrpid;
      }
    return result;
  }
#endif
}

versioned_symbol (libc, __new_msgctl, msgctl, GLIBC_2_2);
