/* Get current working directory.  NaCl version.
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
#include <limits.h>
#include <stdlib.h>
#include <nacl-interfaces.h>

/* Get the pathname of the current working directory,
   and put it in SIZE bytes of BUF.  Returns NULL if the
   directory couldn't be determined or SIZE was too small.
   If successful, returns BUF.  In GNU, if BUF is NULL,
   an array is allocated with `malloc'; the array is SIZE
   bytes long, unless SIZE <= 0, in which case it is as
   big as necessary.  */
char *
__getcwd (char *buf, size_t size)
{
  char *use_buf = buf;

  if (buf == NULL)
    {
      if (size == 0)
        size = PATH_MAX;
      use_buf = malloc (size);
      if (__glibc_unlikely (use_buf == NULL))
        return NULL;
    }

  int error = __nacl_irt_dev_filename.getcwd (use_buf, size);
  if (__glibc_unlikely (error))
    {
      if (use_buf != buf)
        free (use_buf);
      errno = error;
      return NULL;
    }

  return use_buf;
}
weak_alias (__getcwd, getcwd)
