/* Copyright (C) 1991, 1995, 1996 Free Software Foundation, Inc.
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

#include <errno.h>
#include <stddef.h>
#include <dirent.h>

/* Read a directory entry from DIRP, store result in ENTRY and return
   pointer to result in *RESULT.  */
int
__readdir_r (DIR *dirp, struct dirent *entry, struct dirent **result)
{
  __set_errno (ENOSYS);
  *result = NULL;
  return -1;
}
weak_alias (__readdir_r, readdir_r)

stub_warning (readdir_r)
