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

#include <sys/sem.h>

/* Attach the shared memory segment associated with SHMID to the data
   segment of the calling process.  SHMADDR and SHMFLG determine how
   and where the segment is attached.  */

char *
shmat (shmid, shmaddr, shmflg)
     int shmid;
     char *shmaddr;
     int shmflg;
{
  int retval;
  unsigned long raddr;

  retval = __ipc (IPCOP_shmat, shmid, shmflg, (int) &raddr, shmaddr);
  return retval < 0 ? (char *) retval : (char *) raddr;
}
