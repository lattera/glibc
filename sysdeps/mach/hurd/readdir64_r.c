/* Copyright (C) 2001 Free Software Foundation, Inc.
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

#include <dirent.h>
#include <limits.h>
#include <stddef.h>
#include <string.h>

/* Read a directory entry from DIRP.  */
int
__readdir64_r (DIR *dirp, struct dirent64 *entry, struct dirent64 **result)
{
  struct dirent *result32;
  union
  {
    struct dirent d;
    char b[offsetof (struct dirent, d_name) + UCHAR_MAX + 1];
  } u;
  int err;

  err = __readdir_r (dirp, &u.d, &result32);
  if (result32)
    {
      entry->d_fileno = result32->d_fileno;
      entry->d_reclen = result32->d_reclen;
      entry->d_type = result32->d_type;
      entry->d_namlen = result32->d_namlen;
      memcpy (entry->d_name, result32->d_name, result32->d_namlen + 1);
      *result = entry;
    }
  else
    *result = NULL;
      
  return err;
}

weak_alias(__readdir64_r, readdir64_r)
