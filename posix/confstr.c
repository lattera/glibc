/* Copyright (C) 1991 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <ansidecl.h>
#include <stddef.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <confstr.h>

/* If BUF is not NULL, fill in at most LEN characters of BUF
   with the value corresponding to NAME.  Return the number
   of characters required to hold NAME's entire value.  */
size_t
DEFUN(confstr, (name, buf, len),
      int name AND char *buf AND size_t len)
{
  CONST char *string;
  size_t string_len;

  switch (name)
    {
    case _CS_PATH:
      {
	static CONST char cs_path[] = CS_PATH;
	string = cs_path;
	string_len = sizeof(cs_path);
      }
      break;

    default:
      errno = EINVAL;
      return 0;
    }

  if (buf != NULL)
    (void) strncpy(buf, string, len);
  return string_len;
}
