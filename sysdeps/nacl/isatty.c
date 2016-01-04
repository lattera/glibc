/* Determine if a file descriptor refers to a terminal.  NaCl version.
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

#include <errno.h>
#include <unistd.h>
#include <nacl-interfaces.h>

/* Return 1 if FD is a terminal, 0 if not.  */
int
__isatty (int fd)
{
  int result;
  int error = __nacl_irt_dev_fdio.isatty (fd, &result);
  if (error == 0)
    {
      if (result)
	return 1;
      error = ENOTTY;
    }
  errno = error;
  return 0;
}
weak_alias (__isatty, isatty)
