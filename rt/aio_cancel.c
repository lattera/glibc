/* Cancel requests associated with given file descriptor.  Stub version.
   Copyright (C) 2001-2018 Free Software Foundation, Inc.
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
   implementation of aio_cancel and aio_cancel64 are identical and so
   we want to avoid code duplication by using aliases.  But gcc sees
   the different parameter lists and prints a warning.  We define here
   a function so that aio_cancel64 has no prototype.  */
#define aio_cancel64 XXX
#include <aio.h>
/* And undo the hack.  */
#undef aio_cancel64

#include <errno.h>

int
aio_cancel (int fildes, struct aiocb *aiocbp)
{
  __set_errno (ENOSYS);
  return -1;
}

weak_alias (aio_cancel, aio_cancel64)

stub_warning (aio_cancel)
stub_warning (aio_cancel64)
