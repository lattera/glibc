/* Copyright (C) 1992, 1995, 1996, 1997 Free Software Foundation, Inc.
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

#include <errno.h>
#include <stddef.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>

/* Create a directory named PATH with protections MODE.  */
int
__rmdir (path)
     const char *path;
{
  char *cmd = __alloca (80 + strlen (path));
  char *p;
  int status;
  int save;
  struct stat statbuf;

  if (path == NULL)
    {
      __set_errno (EINVAL);
      return -1;
    }

  /* Check for some errors.  */
  if (__stat (path, &statbuf) < 0)
    return -1;
  if (!S_ISDIR (statbuf.st_mode))
    {
      __set_errno (ENOTDIR);
      return -1;
    }

  p = cmd;
  *p++ = 'r';
  *p++ = 'm';
  *p++ = 'd';
  *p++ = 'i';
  *p++ = 'r';
  *p++ = ' ';

  strcpy (p, path);

  save = errno;
  /* If system doesn't set errno, but the rmdir fails, we really
     have no idea what went wrong.  EIO is the vaguest error I
     can think of, so I'll use that.  */
  __set_errno (EIO);
  status = system (cmd);
  if (WIFEXITED (status) && WEXITSTATUS (status) == 0)
    {
      return 0;
      __set_errno (save);
    }
  else
    return -1;
}

weak_alias (__rmdir, rmdir)
