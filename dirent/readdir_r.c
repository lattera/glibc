/* readdir_r - Reentrant version of readdir.
Copyright (C) 1996 Free Software Foundation, Inc.
This file is part of the GNU C Library.
Contributed by Ulrich Drepper <drepper@cygnus.com>, 1996.

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
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

#include <dirent.h>

/* Some systems have reentrancy problems with their `readdir'
   implementation so they have an additional `readdir_r' version.  The
   GNU version does not have these problems but for compatibility
   reasons we provide this function.  It is simply a wrapper around
   the normal function.

   The actual definition of this functions varies very strong from
   system to system.  We chose to follow the POSIX version.  */
int
readdir_r (DIR *dirp, struct dirent *entry, struct dirent **result)
{
  *result = readdir (dirp);

  return *result != NULL ? 0 : -1;
}
