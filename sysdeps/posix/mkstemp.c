/* Copyright (C) 1991, 1992 Free Software Foundation, Inc.
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
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

/* Generate a unique temporary file name from TEMPLATE.
   The last six characters of TEMPLATE must be "XXXXXX";
   they are replaced with a string that makes the filename unique.
   Returns a file descriptor open on the file for reading and writing.  */
int
DEFUN(mkstemp, (template), char *template)
{
  static CONST char letters[]
    = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  size_t len;
  size_t i;

  len = strlen (template);
  if (len < 6 || strcmp (&template[len - 6], "XXXXXX"))
    {
      errno = EINVAL;
      return -1;
    }

  if (sprintf (&template[len - 5], "%.5u",
	       (unsigned int) getpid () % 100000) != 5)
    /* Inconceivable lossage.  */
    return -1;

  for (i = 0; i < sizeof (letters); ++i)
    {
      int fd;

      template[len - 6] = letters[i];

      fd = open (template, O_RDWR|O_CREAT|O_EXCL, 0666);
      if (fd >= 0)
	return fd;
    }

  /* We return the null string if we can't find a unique file name.  */
  template[0] = '\0';
  return -1;
}
