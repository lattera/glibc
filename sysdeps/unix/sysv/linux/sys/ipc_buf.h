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

#ifndef _SYS_IPC_BUF_H
#define _SYS_IPC_BUF_H

#include <features.h>
#include <sys/types.h>

/* Mode bits for `msgget', `semget', and `shmget'.  */
#define IPC_CREAT	01000		/* create key if key does not exist */
#define IPC_EXCL	02000		/* fail if key exists */
#define IPC_NOWAIT	04000		/* return error on wait */

/* Control commands for `msgctl', `semctl', and `shmctl'.  */
#define IPC_RMID	0		/* remove identifier */
#define IPC_SET		1		/* set `ipc_perm' options */
#define IPC_STAT	2		/* get `ipc_perm' options */
#define IPC_INFO	3		/* see ipcs */


__BEGIN_DECLS

/* Special key values.  */
#define IPC_PRIVATE	((key_t) 0)	/* private key */


/* Data structure used to pass permission information to IPC operations.  */
struct ipc_perm
{
  key_t __key;				/* key */
  __uid_t uid;				/* owner's user ID */
  __gid_t gid;				/* owner's group ID */
  __uid_t cuid;				/* creator's user ID */
  __gid_t cgid;				/* creator's group ID */
  __mode_t mode;			/* read/write permission */
  unsigned short int __seq;		/* sequence number */
};


/* Kludge to work around Linux' restriction of only up to five
   arguments to a system call.  */
struct ipc_kludge
{
  void *msgp;
  long msgtyp;
};

/* The actual system call: all functions are multiplexed by this.  */
extern int __ipc __P ((int __call, int __first, int __second, int __third,
		       void *__ptr));

/* The codes for the functions to use the multiplexer `__ipc'.  */
#define IPCOP_semop	 1
#define IPCOP_semget	 2
#define IPCOP_semctl	 3
#define IPCOP_msgsnd	11
#define IPCOP_msgrcv	12
#define IPCOP_msgget	13
#define IPCOP_msgctl	14
#define IPCOP_shmat	21
#define IPCOP_shmdt	22
#define IPCOP_shmget	23
#define IPCOP_shmctl	24

__END_DECLS

#endif /* sys/ipc_buf.h */
