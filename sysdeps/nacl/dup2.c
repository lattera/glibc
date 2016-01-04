/* Duplicate a file descriptor to a chosen number.  NaCl version.
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


/* Duplicate FD to FD2, closing the old FD2 and making FD2 be
   open the same file as FD is.  Return FD2 or -1.  */
int
__dup2 (int fd, int fd2)
{
  return NACL_CALL (__nacl_irt_fdio.dup2 (fd, fd2), fd2);
}
libc_hidden_def (__dup2)
weak_alias (__dup2, dup2)
