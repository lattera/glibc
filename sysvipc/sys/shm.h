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

#ifndef _SYS_SHM_H
#define _SYS_SHM_H

#include <features.h>
#include <unistd.h>		/* for `getpagesize' declaration */
#include <sys/types.h>

/* Get common definition of System V style IPC.  */
#include <sys/ipc.h>

/* Get system dependent definition of `struct shmid_ds' and more.  */
#include <sys/shm_buf.h>


/* Segment low boundary address multiple.  */
#define SHMLBA		getpagesize ()

__BEGIN_DECLS

/* The following System V style IPC functions implement a shared memory
   facility.  The definition is found in XPG2.  */

/* Shared memory control operation.  */
extern int shmctl __P ((int __shmid, int __cmd, struct shmid_ds *__buf));

/* Get shared memory segment.  */
extern int shmget __P ((key_t __key, int __size, int __shmflg));

/* Attach shared memory segment.  */
extern int shmat __P ((int __shmid, char *__shmaddr, int __shmflg));

/* Detach shared memory segment.  */
extern int shmdt __P ((char *__shmaddr));

__END_DECLS

#endif /* sys/shm.h */
