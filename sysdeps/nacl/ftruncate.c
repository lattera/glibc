/* Truncate a file.  NaCl version.
   Copyright (C) 2015-2016 Free Software Foundation, Inc.
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

#include <unistd.h>
#include <nacl-interfaces.h>

/* Truncate the file referenced by FD to LENGTH bytes.  */
int
__ftruncate (int fd, off_t length)
{
  return NACL_CALL (__nacl_irt_dev_fdio.ftruncate (fd, length), 0);
}
weak_alias (__ftruncate, ftruncate)

/* ftruncate64 is the same as ftruncate.  */
strong_alias (__ftruncate, __ftruncate64)
weak_alias (__ftruncate64, ftruncate64)
