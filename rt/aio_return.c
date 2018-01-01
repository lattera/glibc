/* Return exit value of asynchronous I/O request.
   Copyright (C) 1997-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1997.

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
   implementation of aio_return and aio_return64 are identical and so
   we want to avoid code duplication by using aliases.  But gcc sees
   the different parameter lists and prints a warning.  We define here
   a function so that aio_return64 has no prototype.  */
#define aio_return64 XXX
#include <aio.h>
/* And undo the hack.  */
#undef aio_return64


ssize_t
aio_return (struct aiocb *aiocbp)
{
  return aiocbp->__return_value;
}

weak_alias (aio_return, aio_return64)
