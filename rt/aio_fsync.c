/* Synchronize I/O in given file descriptor.  Stub version.
   Copyright (C) 2001 Free Software Foundation, Inc.
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


/* We use an UGLY hack to prevent gcc from finding us cheating.  The
   implementation of aio_fsync and aio_fsync64 are identical and so
   we want to avoid code duplication by using aliases.  But gcc sees
   the different parameter lists and prints a warning.  We define here
   a function so that aio_fsync64 has no prototype.  */
#define aio_fsync64 XXX
#include <aio.h>
/* And undo the hack.  */
#undef aio_fsync64

#include <errno.h>
#include <fcntl.h>

int
aio_fsync (int op, struct aiocb *aiocbp)
{
  if (op != O_SYNC && op != O_DSYNC)
    {
      __set_errno (EINVAL);
      return -1;
    }

  __set_errno (ENOSYS);
  return -1;
}

weak_alias (aio_fsync, aio_fsync64)

stub_warning (aio_fsync)
stub_warning (aio_fsync64)
