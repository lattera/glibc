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
#include <stdio.h>
#include <sys/types.h>
#include <grp.h>

/* Search for an entry with a matching group ID.  */
struct group *
DEFUN(getgrgid, (gid), register gid_t gid)
{
  static PTR info = NULL;
  register FILE *stream;
  register struct group *g;

  if (info == NULL)
    {
      info = __grpalloc();
      if (info == NULL)
	return NULL;
    }

  stream = __grpopen();
  if (stream == NULL)
    return NULL;

  while ((g = __grpread(stream, info)) != NULL)
    if (g->gr_gid == (gid_t) gid)
      break;

  (void) fclose(stream);
  return g;
}
