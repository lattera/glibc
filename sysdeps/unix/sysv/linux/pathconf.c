/* Get file-specific information about a file.  Linux version.
   Copyright (C) 1991,95,96,98,99,2000,2001,2002 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include "pathconf.h"

static long int posix_pathconf (const char *file, int name);

/* Define this first, so it can be inlined.  */
#define __pathconf static posix_pathconf
#include <sysdeps/posix/pathconf.c>


/* Get file-specific information about FILE.  */
long int
__pathconf (const char *file, int name)
{
  struct statfs fsbuf;

  switch (name)
    {
    case _PC_LINK_MAX:
      return statfs_link_max (__statfs (file, &fsbuf), &fsbuf);
    case _PC_FILESIZEBITS:
      return statfs_filesize_max (__statfs (file, &fsbuf), &fsbuf);
    default:
      return posix_pathconf (file, name);
    }
}
