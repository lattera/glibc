/* Copyright (C) 1992, 1994, 1995, 1996, 1997 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <stddef.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>

/* Create a directory named PATH with protections MODE.  */
int
__mkdir (path, mode)
     const char *path;
     mode_t mode;
{
  char *cmd = __alloca (80 + strlen (path));
  char *p;
  int status;
  mode_t mask;
  int save;
  struct stat statbuf;

  if (path == NULL)
    {
      __set_errno (EINVAL);
      return -1;
    }

  /* Check for some errors.  */
  if (__stat (path, &statbuf) < 0)
    {
      if (errno != ENOENT)
	return -1;
      /* There is no file by that name.  Good.  */
    }
  else
    {
      __set_errno (EEXIST);
      return -1;
    }

  /* Race condition, but how else to do it?  */
  mask = __umask (0777);
  (void) __umask (mask);

  p = cmd;
  *p++ = 'm';
  *p++ = 'k';
  *p++ = 'd';
  *p++ = 'i';
  *p++ = 'r';
  *p++ = ' ';

  mode &= ~mask;
  *p++ = '-';
  *p++ = 'm';
  *p++ = ' ';
  *p++ = ((mode & 07000) >> 9) + '0';
  *p++ = ((mode & 0700) >> 6) + '0';
  *p++ = ((mode & 070) >> 3) + '0';
  *p++ = ((mode & 07)) + '0';
  *p++ = ' ';

  strcpy (p, path);

  save = errno;
  /* If system doesn't set errno, but the mkdir fails, we really
     have no idea what went wrong.  EIO is the vaguest error I
     can think of, so I'll use that.  */
  __set_errno (EIO);
  status = system (cmd);
  if (WIFEXITED (status) && WEXITSTATUS (status) == 0)
    {
      __set_errno (save);
      return 0;
    }
  else
    return -1;
}

weak_alias (__mkdir, mkdir)
