/* Copyright (C) 1995 Free Software Foundation, Inc.
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
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

#ifndef _SYS_MSG_H
#define _SYS_MSG_H

#include <features.h>
#include <sys/types.h>

/* Get common definition of System V style IPC.  */
#include <sys/ipc.h>

/* Get system dependent definition of `struct msqid_ds' and more.  */
#include <sys/msq_buf.h>

__BEGIN_DECLS

/* The following System V style IPC functions implement a message queue
   system.  The definition is found in XPG2.  */

/* Template for struct to be used as argument for `msgsnd' and `msgrcv'.  */
struct msgbuf
{
  long mtype;			/* type of received/sent message */
  char mtext[1];		/* text of the message */
};


/* Message queue control operation.  */
extern int msgctl __P ((int __msqid, int __cmd, struct msqid_ds *__buf));

/* Get messages queue.  */
extern int msgget __P ((key_t __key, int __msgflg));

/* Receive message from message queue.  */
extern int msgrcv __P ((int __msqid, void *__msgp, size_t __msgsz,
			long __msgtyp, int __msgflg));

/* Send message to message queue.  */
extern int msgsnd __P ((int __msqid, void *__msgp, size_t __msgsz,
			int __msgflg));

__END_DECLS

#endif /* sys/msg.h */
