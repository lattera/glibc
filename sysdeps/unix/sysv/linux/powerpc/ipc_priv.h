/* Old SysV permission definition for Linux.  PowerPC version.
   Copyright (C) 1995-2018 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <sys/ipc.h>  /* For __key_t  */

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

#define SEMCTL_ARG_ADDRESS(__arg) &__arg.array

#define MSGRCV_ARGS(__msgp, __msgtyp) \
  ((long int []){ (long int) __msgp, __msgtyp })

#include <ipc_ops.h>
