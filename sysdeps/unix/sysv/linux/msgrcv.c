/* Copyright (C) 1995-2016 Free Software Foundation, Inc.
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

#include <sysdep-cancel.h>
#include <sys/syscall.h>

/* Kludge to work around Linux' restriction of only up to five
   arguments to a system call.  */
struct ipc_kludge
  {
    void *msgp;
    long int msgtyp;
  };


ssize_t
__libc_msgrcv (int msqid, void *msgp, size_t msgsz, long int msgtyp,
	       int msgflg)
{
  /* The problem here is that Linux' calling convention only allows up to
     fives parameters to a system call.  */
  struct ipc_kludge tmp;

  tmp.msgp = msgp;
  tmp.msgtyp = msgtyp;

  return SYSCALL_CANCEL (ipc, IPCOP_msgrcv, msqid, msgsz, msgflg, &tmp);
}
weak_alias (__libc_msgrcv, msgrcv)
