/* Copyright (C) 1991, 1992, 1993 Free Software Foundation, Inc.
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
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

/* Generate a unique temporary file name from TEMPLATE.
   The last six characters of TEMPLATE must be "XXXXXX";
   they are replaced with a string that makes the filename unique.  */
char *
DEFUN(mktemp, (template), char *template)
{
  static CONST char letters[]
    = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  size_t len;
  size_t i;

  len = strlen (template);
  if (len < 6 || strcmp (&template[len - 6], "XXXXXX"))
    {
      errno = EINVAL;
      return NULL;
    }

  if (sprintf (&template[len - 5], "%.5u",
	       (unsigned int) getpid () % 100000) != 5)
    /* Inconceivable lossage.  */
    return NULL;

  for (i = 0; i < sizeof (letters); ++i)
    {
      struct stat ignored;

      template[len - 6] = letters[i];

      if (stat (template, &ignored) < 0 && errno == ENOENT)
	/* The file does not exist.  So return this name.  */
	return template;
    }

  /* We return the null string if we can't find a unique file name.  */
  template[0] = '\0';
  return template;
}
