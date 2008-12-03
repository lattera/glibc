/* Copyright (C) 1995-1999, 2000, 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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

#include <sys/ipc.h>

#define __IPC_64	0x100

struct __old_ipc_perm
{
  __key_t __key;			/* Key.  */
  unsigned int uid;			/* Owner's user ID.  */
  unsigned int gid;			/* Owner's group ID.  */
  unsigned int cuid;			/* Creator's user ID.  */
  unsigned int cgid;			/* Creator's group ID.  */
  unsigned int mode;			/* Read/write permission.  */
  unsigned short int __seq;		/* Sequence number.  */
};


/* The codes for the functions to use the ipc syscall multiplexer.  */
#define IPCOP_semop	 1
#define IPCOP_semget	 2
#define IPCOP_semctl	 3
#define IPCOP_semtimedop 4
#define IPCOP_msgsnd	11
#define IPCOP_msgrcv	12
#define IPCOP_msgget	13
#define IPCOP_msgctl	14
#define IPCOP_shmat	21
#define IPCOP_shmdt	22
#define IPCOP_shmget	23
#define IPCOP_shmctl	24
