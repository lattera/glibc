/* Copyright (C) 1995, 1997, 1998, 2000 Free Software Foundation, Inc.
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
#include <sys/syscall.h>

#include <bp-checks.h>

/* Kludge to work around Linux' restriction of only up to five
   arguments to a system call.  */
struct ipc_kludge
  {
    void *__unbounded msgp;
    long int msgtyp;
  };


int
msgrcv (msqid, msgp, msgsz, msgtyp, msgflg)
     int msqid;
     void *msgp;
     size_t msgsz;
     long int msgtyp;
     int msgflg;
{
  /* The problem here is that Linux' calling convention only allows up to
     fives parameters to a system call.  */
  struct ipc_kludge tmp;

  tmp.msgp = CHECK_N (msgp, msgsz);
  tmp.msgtyp = msgtyp;

  return INLINE_SYSCALL (ipc, 5, IPCOP_msgrcv, msqid, msgsz, msgflg, __ptrvalue (&tmp));
}
