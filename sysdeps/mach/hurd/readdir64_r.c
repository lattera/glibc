/* Copyright (C) 1998 Free Software Foundation, Inc.
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
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>

/* Read a directory entry from DIRP.  */
int
__readdir64_r (DIR *dirp, struct dirent64 *entry, struct dirent64 **result)
{
  struct dirent ent32, *res32;

  /* XXX the new __dir_readdir64 RPC is not yet implemented */

  /* If __dir_readdir64 failed, then fall back on the old implementation */
  if (readdir_r (dirp, &ent32, &res32))
    return -1;

  if (!res32)
    {
      /* End of directory. */
      *result = 0;
      return 0;
    }

  /* Convert our result from the 32-bit value. */
  memset (entry, 0, sizeof (*entry));
  entry->d_reclen = sizeof (*entry);

  entry->d_ino = res32->d_fileno;
  entry->d_type = res32->d_type;
  memcpy (entry->d_name, res32->d_name, res32->d_namlen);

  *result = entry;
  return 0;
}
weak_alias (__readdir64_r, readdir64_r)
