/* Copyright (C) 1995, 1996, 1997, 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#ifndef _SYS_SEM_H
#define _SYS_SEM_H	1

#include <features.h>

#include <sys/types.h>

/* Get common definition of System V style IPC.  */
#include <sys/ipc.h>

/* Get system dependent definition of `struct semid_ds' and more.  */
#include <bits/sem.h>

/* The following System V style IPC functions implement a semaphore
   handling.  The definition is found in XPG2.  */

/* Structure used for argument to `semop' to describe operations.  */
struct sembuf
{
  short int sem_num;		/* semaphore number */
  short int sem_op;		/* semaphore operation */
  short int sem_flg;		/* operation flag */
};


__BEGIN_DECLS

/* Semaphore control operation.  */
extern int semctl __P ((int __semid, int __semnum, int __cmd, ...));

/* Get semaphore.  */
extern int semget __P ((key_t __key, int __nsems, int __semflg));

/* Operate on semaphore.  */
extern int semop __P ((int __semid, struct sembuf *__sops,
		       unsigned int __nsops));

__END_DECLS

#endif /* sys/sem.h */
