/* Bounded-pointer checking macros for C.
   Copyright (C) 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Greg McGary <greg@mcgary.org>

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

#ifndef _bp_semctl_h_
#define _bp_semctl_h_ 1

#if __BOUNDED_POINTERS__

# define CHECK_SEMCTL(ARGP, SEMID, CMD) check_semctl (ARGP, SEMID, CMD)

union semun *__unbounded
check_semctl (union semun *arg, int semid, int cmd)
{
  int ipc64 = (cmd & __IPC_64);

  switch (cmd & ~__IPC_64)
    {
    case IPC_STAT:
    case IPC_SET:
      (void) CHECK_1 (arg->buf);
      break;

    case GETALL:
    case SETALL:
      {
	struct semid_ds ds;
	union semun un = { .buf = &ds };
	unsigned int length = ~0;

	/* It's unfortunate that we need to make a recursive
	   system call to get the size of the semaphore set...  */
	if (semctl (semid, 0, IPC_STAT | ipc64, un) == 0)
	  length = ds.sem_nsems;
	(void) CHECK_N (arg->array, length);
	break;
      }

    case IPC_INFO:
      (void) CHECK_1 (arg->__buf);
      break;
    }

  return __ptrvalue (arg);
}

#else
# define CHECK_SEMCTL(ARGP, SEMID, CMD) (ARGP)
#endif

#endif /* _bp_semctl_h_ */
