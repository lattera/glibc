/* Copyright (C) 1998-2018 Free Software Foundation, Inc.
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

#include <errno.h>
#include <fcntl.h>
#include <stropts.h>

int
isastream (int fildes)
{
  /* In general we do not have a STREAMS implementation and therefore
     return 0.  But for invalid file descriptors we have to return an
     error.  */
  if (__fcntl (fildes, F_GETFD) < 0)
    return -1;

  /* No STREAM.  */
  return 0;
}
